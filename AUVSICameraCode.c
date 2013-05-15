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

#include "AUVSICameraCode.h"

#define BUFFER_SIZE 80
#define STOP_CYCLE_COUNT 20

/*
   Latest AUVSICameraCode as of 4/26/2013.
   Has three separate threads to:
   1) Wait for image and get from camera
   2) Save image to disk
   3) Wait for telemetry data and save it to txt file

   Telemetry listens from /dev/ttyUSB0

   4/17/2013: Ability to change quality with file input has been added
   4/26/2013: Program now has ability to start and stop shooting with file input as well

   Looks for a start or stop file. Start file must contain number (1-4) indicating Hz to shoot at
   Sends this number as a byte on Serial at /dev/ttyUSB1 (or 0 if stop)
   Byte is character representation of that number, e.g. 0 = 0x30, 1 = 0x31... 4 = 0x34

   Works on Atom system

BUG: if quality is changed during a shoot cycle, weird stuff happens. Avoid doing it.
Addendum: If using the proper front-end file creator on the ground, this should never happen

TODO: Clearly define function error codes
TODO: Ability to change other settings (not priority)
 */

typedef struct CF{
	struct CF* next;
	CameraFile* my_File;
	char* filename;
} CF;

CF *startCF, *endCF;

sem_t FileQSem, NewFileSem;

int serialFDTel, serialFDAPM;

Camera* my_Camera; 
GPContext* my_Context;

static int DLWaitCounter = 0;
static int currentMode = -1;
static int currentQuality = -1;

int main(int argc, char ** argv){
	gp_camera_new(&my_Camera);
	my_Context = gp_context_new();

	int result = gp_camera_init(my_Camera, my_Context);
	printf("\n\nCamera Init Result: %d\n\n", result);

	if(result == -105){
		printf("Camera not found, quitting.\n\n");
		return -105;
	} 

	sem_init(&FileQSem, 0, 1);
	sem_init(&NewFileSem, 0, 0);

	//Open our telemetry serial and our APM serial
	setupSerial();

	//Spawning the three worker threads
	pthread_t telemetryThread, getThread, saveThread;

	pthread_create(&getThread, NULL, GetEvents, NULL);
	pthread_create(&saveThread, NULL, SaveFiles, NULL);
	pthread_create(&telemetryThread, NULL, telemetrySync, NULL);

	pthread_join(getThread, NULL);
	pthread_join(saveThread, NULL);
	pthread_join(telemetryThread, NULL);
}

int setupSerial()
{
	struct termios options;

	serialFDTel = open("/dev/ttyUSB0", O_RDONLY | O_NOCTTY);
	if(serialFDTel == -1)
	{
		perror("Error setting up serial port (ttyUSB0)");
	}
	else{
		tcgetattr(serialFDTel, &options);
		cfsetispeed(&options, B57600);
		cfsetospeed(&options, B57600);
		options.c_cflag |= (CLOCAL |CREAD);
		tcsetattr(serialFDTel, TCSANOW, &options);
	}

	serialFDAPM = open("/dev/ttyUSB1", O_WRONLY | O_NOCTTY);
	if(serialFDAPM == -1){
		perror("Error setting up serial port (ttyUSB1)");
	}
	else{
		tcgetattr(serialFDAPM, &options);
		cfsetispeed(&options, B57600);
		cfsetospeed(&options, B57600);
		options.c_cflag |= (CLOCAL | CREAD);
		tcsetattr(serialFDAPM, TCSANOW, &options);
	}

	if(serialFDTel == -1 || serialFDAPM == -1){
		return -1;
	}
	return 0;
}

void * telemetrySync()
{
	int num_bytes_read = 0;
	char buffer[BUFFER_SIZE];
	int i, j, k = 0;

	char number[10]; //Assumed to be max of 5 character long number + ".txt" + NUL
	memset(number, '\0', 9);

	char line[BUFFER_SIZE];
	int readingString = 0;

	while(1){
		while((num_bytes_read = read(serialFDTel, &buffer, BUFFER_SIZE)) 
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
						fprintf(telemfile, "%s",  line);
						fclose(telemfile);
						memset(number, '\0', 9);
						memset(line, '\0', BUFFER_SIZE-1);
					}
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
//		if(serialFDAPM != -1){ //If APM Serial is setup
			CameraControl();
//		}
		do{
			gp_camera_wait_for_event(my_Camera, 100, &my_Type, &retevent, my_Context);
			if(my_Type == GP_EVENT_FILE_ADDED){
				CameraFilePath* my_FP = (CameraFilePath*)retevent;

				gp_file_new(&my_File);

				gp_camera_file_get(my_Camera, my_FP->folder, my_FP->name, GP_FILE_TYPE_NORMAL, my_File, my_Context);

				AddFile(my_File, my_FP->name);

				sem_post(&NewFileSem);
				free(my_FP);
			}
		} while(--DLWaitCounter > 0);
		DLWaitCounter = 0;
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
			printf("\nFile Saved!\n");
		}
	}
	return NULL;
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

//Looks for and consume four types of files, and acts appropriately
//start.txt: reads a byte from the file, interprets that as Hz freq, sends that byte off on APM Serial
//stop.txt: sends off a byte for 0 Hz on APM Serial
//large.txt: Talks to camera over USB link, tells it to change to large quality
//medium.txt: Talks to camera over USB link, tells it to change to medium quality
int CameraControl(){
	FILE * stopFile = fopen("stop.txt", "r");
	if(stopFile){
		fclose(stopFile);
		remove("stop.txt");

		if(CaptureControl(0x30) == 0){
			DLWaitCounter = STOP_CYCLE_COUNT; //Set the camera file get to cycle STOP_CYCLE_COUNT times, clearing picture buffer (hopefully)
		}
	}
	if(DLWaitCounter == 0){ //We only want to do any of this if we have finished cycling out images (otherwise weird stuff happens)
		FILE * qualFile = fopen("large.txt", "r");
		if(qualFile)
		{
			fclose(qualFile);
			remove("large.txt");

			QualityControl(0);
		}
		qualFile = fopen("medium.txt", "r");
		if(qualFile)
		{
			fclose(qualFile);
			remove("medium.txt");

			QualityControl(2);
		}
		FILE * startFile = fopen("start.txt", "r"); 
		if(startFile){
			char speed;
			fscanf(startFile, "%c", &speed); //Reads a single character out from the file, interpreting it as shoot freq. in Hz (should be 1-4)
			fclose(startFile);
			remove("start.txt");

			CaptureControl(speed);
		}
	}
}

//Send off a byte on APM Serial representing shoot freq. in Hz
int CaptureControl(char mode){
	if(mode == currentMode){ //If we are already in this mode, return
		return -1;
	}
	if(mode < 0x30 || mode > 0x34){ //If we are sending over an invalid freq., return
		return -1;
	}

	currentMode = mode;
	printf("Writing %x\n", mode);
	write(serialFDAPM, &mode, 1); 
	return 0;
}

//Set the camera's quality to a certain choice
//Current Choices are:
// 0: Large Fine JPEG
// 2: Medium Fine JPEG
int QualityControl(int quality){
	char qualString[20];

	if(quality == currentQuality){ //We don't want to take up cycles changing nothing
		return -1; //Could define set error codes, but we don't look at them anyway
	}

	strncpy(qualString, "", sizeof qualString); //Debatably necessary

	if(quality == 0){
		strncpy(qualString, "Large Fine JPEG", sizeof qualString);
	}
	else if(quality == 2){
		strncpy(qualString, "Medium Fine JPEG", sizeof qualString);
	}
	else{	
		return -1;
	}

	CameraWidget* widget,* child;

	if(gp_camera_get_config(my_Camera, &widget, my_Context) != 0){ //Get our current config
		free(widget);
		free(child);
		return -1; //If it fails, we have failed.
	}

	//Widgets in libgphoto act sort of as trees of settings/data
	//We need to find the right child of the top level, and the right child of that, etc.
	//until we get down to the appropriate quality setting widget
	//I already parsed through the tree and found the right child, and so
	//have hard-coded the values to get to this child here
	//Check out gphoto2-widget.c for more info

	gp_widget_get_child(widget, 3, &child);
	widget = child;
	gp_widget_get_child(widget, 0, &child);
	widget = child;

	//Here we change the quality value to whatever we want
	//For some reason, it is saved as a string, not a choice
	//Don't ask me why.
	gp_widget_set_value(widget, qualString);
	gp_widget_get_root(widget, &child);

	if(gp_camera_set_config(my_Camera, child, my_Context) != 0){ //Set the camera's config to our new, modified config
		free(widget);
		free(child);
		return -1;
	}

	free(widget);
	free(child);

	currentQuality = quality; //Remember what quality we currently have
	return 0;
}
