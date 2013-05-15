#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>	

#include "SerialControl.h"

#define BUFFER_SIZE 80

int serialFDTel;

int initTelListener(){
	if(openOPSerial(&serialFDTel) == -1)
	{
		perror("Error setting up Telemetry Serial Port");
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

