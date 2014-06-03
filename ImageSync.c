#include <stdlib.h>
#include <gphoto2/gphoto2.h>
#include <stdio.h>
#include <string.h>

#include <time.h>

#include <semaphore.h>

#include "SharedInfo.h"
#include "TLMSync.h"

//Struct that sets up a linked list node
//Holds CameraFile objects (basically just images in memory) and the filename that this image will be saved to disk with
typedef struct CF{
    struct CF* next;
    CameraFile* my_File;
    char* filename;
} CF;

CF *startCF, *endCF; //Beginning and end of CameraFile Queue

sem_t FileQSem, NewFileSem; //Semaphores for adding to CameraFile Queue and for controlling Save Thread, respectively


int initImageSync(){
    sem_init(&FileQSem, 0, 1); 
    sem_init(&NewFileSem, 0, 0);

    return initCamera();
}

//Add the given CameraFile to the queue with the given filename
//Creates a new CF node, adds to end of CF Queue
int AddFile(CameraFile* file, const char* filename){
    sem_wait(&FileQSem); //Wait for RemoveFile to possibly finish
    CF* newcf = malloc(sizeof(CF));

    if(newcf == NULL){
        sem_post(&FileQSem);
        return -1;
    }

    //Setup new CF object's data
    newcf -> my_File = file;
    newcf -> filename = malloc(strlen(filename)+1);
    strcpy(newcf -> filename, filename);
    newcf -> next = NULL;

    //Append to end of queue
    if(startCF == NULL){
        startCF = newcf;
        endCF = newcf;
    }
    else{
        endCF -> next = newcf;
        endCF = newcf;
    }

    sem_post(&FileQSem); //Allow RemoveFile to run
    return 0;
}

//Remove last node from queue and return the node
CF* RemoveFile(){
    sem_wait(&FileQSem); //Wait for AddFile to possibly finish

    if(startCF == NULL){
        sem_post(&FileQSem);
        return NULL;
    }

    CF* retcf = startCF;
    startCF = startCF -> next;

    sem_post(&FileQSem); //Allow AddFile to run

    return retcf;
}

//GET THREAD
//Gets Files over USB. Camera -> Memory
void * GetEvents(){
    CameraFile* my_File;
    CameraEventType my_Type;
    void* retevent;
    struct timespec time;
    char filename[40];
    while(1){
        CameraControl();
        do{
            //Wait for camera to fire an event. Most are useless/not used by us
            //When a picture is taken (triggered on camera), FILE_ADDED event fired
            //retevent will contain a CameraFilePath object with name/loc on camera of file
            //Timeout is 100 <units, probably ms>
            gp_camera_wait_for_event(getMyCamera(), 100, &my_Type, &retevent, getMyContext());
            if(my_Type == GP_EVENT_FILE_ADDED){
                CameraFilePath* my_FP = (CameraFilePath*)retevent;

                gp_file_new(&my_File);

                //Get the file from the given location to memory
                gp_camera_file_get(getMyCamera(), my_FP->folder, my_FP->name, GP_FILE_TYPE_NORMAL, my_File, getMyContext());

                //Get time in ms since Epoch -> used for filename for Skynet purposes
                clock_gettime(CLOCK_REALTIME, &time);
                long long time_millis = time.tv_sec * 1000 + time.tv_nsec / 1000000; 
                sprintf(filename, "%llu.jpg", time_millis);

                AddFile(my_File, filename);

                //Save the last obtained Telemetry/GPS to disk, with the same filename we just used
                saveLast(time_millis);

                //Tell the Save Thread that there is a new CameraFile enqueued
                sem_post(&NewFileSem);
                free(my_FP);
            }
        } while(decrementWaitCounter() > 0); 
        //WaitCounter is a variable used to cycle through images possibly still on the camera
        //Used to cycle out images before switching quality, see CameraControl.c
        setWaitCounter(0); 
    }
    return NULL;
}

//SAVE THREAD
//Saves Files from memory -> disk
void * SaveFiles(){
    while(1){
        //Wait for Get Thread to tell us we have at least one file waiting
        //Put here to avoid busy waiting
        sem_wait(&NewFileSem);
        CF* my_CF = RemoveFile(); //Dequeue
        if(my_CF != NULL){
            gp_file_save(my_CF -> my_File, my_CF -> filename);
            gp_file_free(my_CF -> my_File);
            free(my_CF -> filename);
            free(my_CF);
            //			printf("\nFile Saved!\n");
        }
    }
    return NULL;
}
