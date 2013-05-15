#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <semaphore.h>
#include <pthread.h>
#include <termios.h>

#include "SerialControl.h"

#define GPS_BUFFER_SIZE 50  	

sem_t currAccess;

char *currGPS; 
int serialFDGPS, useGPS = 0; 

int initGPSListener(){
	sem_init(&currAccess, 0, 0);
	
	if((currGPS = malloc(GPS_BUFFER_SIZE + 1)) == -1){
		return -2;
	}
	
	memset(currGPS, '\0', GPS_BUFFER_SIZE + 1);

	if(openGPSSerial(&serialFDGPS) == -1)
	{
		perror("Error setting up GPS Serial Port");
		return -1;
	}

	return 0;
}

char * getLastGPS(){
	sem_wait(&currAccess);
	char* retGPS = malloc(GPS_BUFFER_SIZE + 1);
	strncpy(retGPS, currGPS, GPS_BUFFER_SIZE);
	sem_post(&currAccess);
	return retGPS;
}

int checkChecksum(unsigned char* packet, int length){
	unsigned char CK_A = packet[length - 2];
	unsigned char CK_B = packet[length - 1];

	unsigned char CK_A_COMP = 0, CK_B_COMP = 0;
	int i;
	for(i = 0; i < length - 2; i++){
		CK_A_COMP += packet[i];
		CK_B_COMP += CK_A_COMP;
	}

	return (CK_A_COMP == CK_A && CK_B_COMP == CK_B)?0:-1;
}

int getPacket(unsigned char* output, int length){
	unsigned char buffer[length];
	int bytesRead = 0, totalRead = 0;
	int outputIndex = 0;
	while(bytesRead < length){
		bytesRead = read(serialFDGPS, &buffer, length - totalRead);
		totalRead += bytesRead;

		int i;
		for(i = 0; i < bytesRead; i++, outputIndex++){
			output[outputIndex] = buffer[i];
		}
	}
	return 0;
}

int getStatus(){
	unsigned char packet[20];
	packet[0] = 0x01;
	packet[1] = 0x03;
	getPacket(packet + 2, 20);
	
	int i;
	for(i = 0; i < 20; i++){
		printf("%02x ", packet[i]);
	}
	printf("\n\n");
	
	if(checkChecksum(packet, 20)){
		if(packet[4] == 0x03 && (packet[5] & 0x03) == 0x03){
			useGPS = 1;
		}
		else{
			useGPS = 0;
		}
	}
	return 0;
}

int getGPS(){
	unsigned char packet[34];
	packet[0] = 0x01;
	packet[1] = 0x02;
	getPacket(packet + 2, 34);
	
	int i;
	for(i = 0; i < 34; i++){
		printf("%02x ", packet[i]);
	}
	printf("\n\n");

	if(checkChecksum(packet, 34) && useGPS){
		sem_wait(&currAccess);
		memset(currGPS, '\0', GPS_BUFFER_SIZE + 1);
		
		int i;
		for(i = 0; i < 28; i++){ //POSLLH has payload size of 28
			currGPS[i] = packet[i+4]; //Skipping first 4 bytes of packet
		}
		sem_post(&currAccess);
	}
	return 0;
}

void* GPSListenControl(){
	unsigned char nextByte;
	while(1){
		read(serialFDGPS, &nextByte, 1);
		printf("NB: %x\n", nextByte);
		if(nextByte == 0xB5){ //First byte of packet
			printf("First Byte!\n");
			read(serialFDGPS, &nextByte, 1);
			if(nextByte == 0x62){ //Second byte of packet
				printf("Second Byte!\n");
				read(serialFDGPS, &nextByte, 1);
				if(nextByte == 0x01){ //Class byte = NAV
					printf("Third Byte!\n");
					read(serialFDGPS, &nextByte, 1);
					printf("Fourth Byte = %02x!\n", nextByte);

					switch(nextByte){
						case 0x02: //POSLLH
							getGPS();
							break;
						case 0x03: //STATUS
							getStatus();
							break;
						default: break;
					}
				}
			}
		}
	}
	return NULL;
}
/*
int main(){
	initGPSListener();
	//GPSListenControl();
	getGPS();
	getStatus();
}*/
