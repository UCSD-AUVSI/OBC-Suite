#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>	

#include <semaphore.h>

#include "SerialControl.h"
#include "GPSSync.h"

#define NAME_SIZE 9
#define BUFFER_SIZE 80
#define LINE_SIZE 150

sem_t currAccess;

char *currTelName, *currTelData;
int serialFDTel, currTelNumber = 0;

int initTelListener(){
	sem_init(&currAccess, 0, 1);
	
	currTelName = malloc(NAME_SIZE + 1);
	memset(currTelName, '\0', NAME_SIZE + 1);
	currTelData = malloc(LINE_SIZE + 1);
	memset(currTelData, '\0', LINE_SIZE + 1);

	if(openOPSerial(&serialFDTel) == -1)
	{
		perror("Error setting up Telemetry Serial Port");
		return -1;
	}
	return 0;
}

void saveLast(){
	sem_wait(&currAccess);
	
	sprintf(currTelName, "%04d.txt", currTelNumber);
	currTelNumber++;

	FILE * telemfile = fopen(currTelName, "w");
	fprintf(telemfile, "%s", currTelData);
	fclose(telemfile);
	
	sem_post(&currAccess);
}

void * telemetrySync()
{
	int num_bytes_read = 0;
	int i, j, k = 0;

	char buffer[BUFFER_SIZE + 1];
	
	char number[NAME_SIZE + 1]; //Assumed to be max of 5 character long number + ".txt" + NUL
	memset(number, '\0', NAME_SIZE + 1);

	char line[LINE_SIZE + 1];
	int readingString = 0;

	while(1){
		num_bytes_read = read(serialFDTel, &buffer, BUFFER_SIZE); 
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
					
					char* GPSData = getLastGPS();
					strcat(line, GPSData);
					free(GPSData);

					sem_wait(&currAccess);
//					strncpy(currTelName, number, NAME_SIZE);
					strncpy(currTelData, line, LINE_SIZE);
					sem_post(&currAccess);
					
					memset(number, '\0', NAME_SIZE + 1);
					memset(line, '\0', LINE_SIZE + 1);
				}
			}
		}
	}
	return NULL;
}
