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

#define NAME_SIZE 40
#define BUFFER_SIZE 80
#define LINE_SIZE 150

sem_t currAccess;

char *currTLMName, *currTLMData;
int serialFDTLM;

int initTLMListener(){
    sem_init(&currAccess, 0, 1);

    currTLMName = malloc(NAME_SIZE + 1);
    memset(currTLMName, '\0', NAME_SIZE + 1);
    currTLMData = malloc(LINE_SIZE + 1);
    memset(currTLMData, '\0', LINE_SIZE + 1);

    if(getTLMSerial(&serialFDTLM) == -1)
    {
        perror("Error setting up TLM Serial Port");
        return -1;
    }
    return 0;
}

void saveLast(long long time){
    sem_wait(&currAccess);

    sprintf(currTLMName, "%llu.txt", time);

    printf("TLM: %s\n", currTLMData);
    FILE * TLMfile = fopen(currTLMName, "w");
    fprintf(TLMfile, "%s", currTLMData);
    fclose(TLMfile);

    sem_post(&currAccess);
}

void * TLMSync()
{
    printf("Starting Telemetry Sync\n");
    int num_bytes_read = 0;
    int i, j, k = 0;

    char buffer[BUFFER_SIZE + 1];
    char line[LINE_SIZE + 1];

    while(1){
        num_bytes_read = read(serialFDTLM, &buffer, BUFFER_SIZE); 
        for(i = 0; i < num_bytes_read; i++){
            if(buffer[i] == '\n')   {
                j=0;
                sem_wait(&currAccess);
                strncpy(currTLMData, line, LINE_SIZE);
                sem_post(&currAccess);

                memset(line, '\0', LINE_SIZE + 1);
            }
            else    {
                line[j]=buffer[i];
                j++;
            }
        }
    }
    return NULL;
}
