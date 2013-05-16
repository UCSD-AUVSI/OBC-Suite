#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

#include <semaphore.h>
#include <pthread.h>
#include <termios.h>

#include "SerialControl.h"
#define GPS_BUFFER_SIZE 80  	

sem_t currAccess;

char *currGPS; 
int serialFDGPS, useGPS = 0; 

int initGPSListener(){
	sem_init(&currAccess, 0, 1);
	
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

int charToInt(unsigned char* input){
	unsigned int retval;
	retval = (unsigned int)(input[0]);
	retval |= ((unsigned int)(input[1]) << 8);
	retval |= ((unsigned int)(input[2]) << 16);
	retval |= ((unsigned int)(input[3]) << 24);
	
	return (int)retval;
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
	while(totalRead < length){
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
	unsigned char packet[22];
	packet[0] = 0x01;
	packet[1] = 0x03;
	getPacket(packet + 2, 20);
	
/*	int i;
	for(i = 0; i < 20; i++){
		printf("%02x ", packet[i]);
	}
	printf("\n\n");
*/
	if(checkChecksum(packet, 22) == 0){
		if((packet[8] == 0x02 || packet[8] == 0x03 || packet[8] == 0x04) && (packet[9] & 0x01) == 0x01){//gpsFixOk && 2D/3D/GPS+DeadReckoning
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
	getPacket(packet + 2, 32);
	
/*	int i;
	for(i = 8; i < 24; i++){
		printf("%d: %02x ", i, packet[i]);
	}
	printf("\n\n");
*/
	if(checkChecksum(packet, 34) == 0){
		sem_wait(&currAccess);
		
		memset(currGPS, '\0', GPS_BUFFER_SIZE + 1);
		sprintf(currGPS, " %f %f %d %d", charToInt(packet + 12)/pow(10,7), charToInt(packet + 8)/pow(10,7), charToInt(packet + 20), useGPS);

		sem_post(&currAccess);
	}
	return 0;
}

void* GPSListenControl(){
	unsigned char nextByte;
	while(1){
		read(serialFDGPS, &nextByte, 1);
		if(nextByte == 0xB5){ //First byte of packet
			read(serialFDGPS, &nextByte, 1);
			if(nextByte == 0x62){ //Second byte of packet
				read(serialFDGPS, &nextByte, 1);
				if(nextByte == 0x01){ //Class byte = NAV
					read(serialFDGPS, &nextByte, 1);

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
	GPSListenControl();
}*/
