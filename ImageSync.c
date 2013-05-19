#include <stdlib.h>
#include <gphoto2/gphoto2.h>
#include <stdio.h>
#include <string.h>

#include <semaphore.h>

#include "CameraInfo.h"

typedef struct CF{
	struct CF* next;
	CameraFile* my_File;
	char* filename;
} CF;

CF *startCF, *endCF;

sem_t FileQSem, NewFileSem;

int initImageSync(){
	sem_init(&FileQSem, 0, 1);
	sem_init(&NewFileSem, 0, 0);
	
	return initCamera();
}

int AddFile(CameraFile* file, const char* filename){
	sem_wait(&FileQSem);
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

	sem_post(&FileQSem);
	return 0;
}

CF* RemoveFile(){
	sem_wait(&FileQSem);

	if(startCF == NULL){
		sem_post(&FileQSem);
		return NULL;
	}

	CF* retcf = startCF;
	startCF = startCF -> next;

	sem_post(&FileQSem);

	return retcf;
}

void * GetEvents(){
	CameraFile* my_File;
	CameraEventType my_Type;
	void* retevent;
	while(1){
		CameraControl();
		do{
			gp_camera_wait_for_event(getMyCamera(), 100, &my_Type, &retevent, getMyContext());
			if(my_Type == GP_EVENT_FILE_ADDED){
				CameraFilePath* my_FP = (CameraFilePath*)retevent;

				gp_file_new(&my_File);

				gp_camera_file_get(getMyCamera(), my_FP->folder, my_FP->name, GP_FILE_TYPE_NORMAL, my_File, getMyContext());

				AddFile(my_File, my_FP->name);

				sem_post(&NewFileSem);
				free(my_FP);
			}
		} while(decrementWaitCounter() > 0);
		setWaitCounter(0);
	}
	return NULL;
}

void * SaveFiles(){
	while(1){
		sem_wait(&NewFileSem);
		CF* my_CF = RemoveFile();
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
