#include <stdio.h>
#include <stdlib.h>
#include <gphoto2/gphoto2.h>
#include <string.h>
#include <time.h>

#include <semaphore.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>	

#define BUFFER_SIZE 80

typedef struct CFP{
	struct CFP* next;
	CameraFilePath* my_FP;
} CFP;

typedef struct CF{
	struct CF* next;
	CameraFile* my_File;
	char* filename;
} CF;

int telemetryCreate();
void * telemetrySync();

void * GetEvents();
void * SaveFiles();
int AddEvent();
CameraFilePath* RemoveEvent();
int AddFile();
CF* RemoveFile();

CFP *startCFP, *endCFP;
CF *startCF, *endCF;

sem_t EventQSem, FileQSem, NewFileSem;

int serialFd;
char MinigunMode = 0;

Camera* my_Camera; 
GPContext* my_Context;

int main(int argc, char ** argv){
	gp_camera_new(&my_Camera);
	my_Context = gp_context_new();
    
	int result = gp_camera_init(my_Camera, my_Context);
	printf("\n\nCamera Init Result: %d\n\n", result);
   	
	if(result == -105){
		printf("Camera not found, quitting.\n\n");
		return-105;
	} 
	
	sem_init(&EventQSem, 0, 1);
	sem_init(&FileQSem, 0, 1);
	sem_init(&NewFileSem, 0, 0);

	telemetryCreate();

    pthread_t telemetryThread, getThread, saveThread;

    pthread_create(&telemetryThread, NULL, telemetrySync, NULL);
    pthread_create(&getThread, NULL, GetEvents, NULL);
    pthread_create(&saveThread, NULL, SaveFiles, NULL);
	
    pthread_join(telemetryThread, NULL);
    pthread_join(getThread, NULL);
    pthread_join(saveThread, NULL);
}

int telemetryCreate()
{
	serialFd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
	if(serialFd == -1)
	{
		perror("Error setting up serial port - ");
		return -1;
	}
	struct termios options;
	tcgetattr(serialFd, &options);
	cfsetispeed(&options, B57600);
	cfsetospeed(&options, B57600);
	options.c_cflag |= (CLOCAL |CREAD);
	tcsetattr(serialFd, TCSANOW, &options);
	return 0;
}

void * telemetrySync()
{
	int num_bytes_read = 0;
	char buffer[BUFFER_SIZE];
	int i, j, k = 0;
	char number[9]; //Assumed to be max of 5 character long number + ".txt"
	char line[BUFFER_SIZE];
	int readingString = 0;
	while( (num_bytes_read = read(serialFd, &buffer, BUFFER_SIZE)) 
		   != 0 )
	{
		for(i = 0; i < num_bytes_read; i++){
			if(!readingString){
				if(buffer[i] != ' '){
					number[j] = buffer[i];
					j++;
				}
				else{
					readingString = 1;
					j = 0;
				}
			}
			else if(readingString){
				if(buffer[i] != '\n'){
					line[k] = buffer[i];
					k++;
				}
				else{
					readingString = 0;
					k = 0;

					strcat(number, ".txt");
					FILE * telemfile = fopen(number, "w");
					fprintf(telemfile, line);
					fclose(telemfile);
					memset(number, '\0', 9);
					memset(line, '\0', BUFFER_SIZE);
				}
			}
		}
	}
	return NULL;
}

void * GetEvents(){
    CameraFile* my_File;
    CameraEventType my_Type;
    void* retevent;
	while(1){
       	FILE * minigunFile = fopen("minigunon.txt", "r");
        if(minigunFile)
        {
            fclose(minigunFile);
            remove("minigunon.txt");
            MinigunMode = 1;
        }
        minigunFile = fopen("minigunoff.txt", "r");
        if(minigunFile)
        {
            fclose(minigunFile);
            remove("minigunoff.txt");
            MinigunMode = 0;
       	}
		
		gp_camera_wait_for_event(my_Camera, 100, &my_Type, &retevent, my_Context);
		if(my_Type == GP_EVENT_FILE_ADDED){
			CameraFilePath* my_FP_ptr = (CameraFilePath*)retevent;
			AddEvent(my_FP_ptr);	
		}
		if(!MinigunMode){
			while(startCFP != NULL){
				gp_file_new(&my_File);
				
				CameraFilePath* my_FP = RemoveEvent();
				if(gp_camera_file_get(my_Camera, my_FP->folder, my_FP->name, GP_FILE_TYPE_NORMAL, my_File, my_Context) != 0)//;
					printf("Failed\n");
				
				AddFile(my_File, my_FP->name);
				
				sem_post(&NewFileSem);
				free(my_FP);
			}
		}
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
			free(my_CF);
			printf("\nFile Saved!\n\n");
		}
	}
	return NULL;
}

int AddEvent(CameraFilePath* fp){
	sem_wait(&EventQSem);
	CFP* newcfp = malloc(sizeof(CFP));
	
	if(newcfp == NULL){
		sem_post(&EventQSem);
		return -1;
	}

	newcfp -> my_FP = fp;
	newcfp -> next = NULL;
	if(startCFP == NULL){
		startCFP = newcfp;
		endCFP = newcfp;
	}
	else{
		endCFP -> next = newcfp;
		endCFP = newcfp;
	}

	sem_post(&EventQSem);

	return 0;
}

CameraFilePath* RemoveEvent(){
	sem_wait(&EventQSem);

	if(startCFP == NULL){
		sem_post(&EventQSem);
		return NULL;
	}
	
	CameraFilePath* retcfp = startCFP -> my_FP;
	CFP* freecfp = startCFP;

	startCFP = startCFP -> next;
	
	free(freecfp);
	
	sem_post(&EventQSem);

	return retcfp;
}

int AddFile(CameraFile* file, const char* filename){
	sem_wait(&FileQSem);
	CF* newcf = malloc(sizeof(CF));
	
	if(newcf == NULL){
		sem_post(&FileQSem);
		return -1;
	}

	newcf -> my_File = file;
	newcf -> filename = malloc(sizeof(filename));
	strcpy(newcf -> filename, filename);
	newcf -> next = NULL;
	
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
