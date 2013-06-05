#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <gphoto2/gphoto2.h>

#include "TelemetrySync.h"
#include "CameraControl.h"
#include "GPSSync.h"
#include "ImageSync.h"
#include "SharedInfo.h"

#define DEFAULT_OP "0"
#define DEFAULT_APM "2"
#define DEFAULT_GPS "1"

/*
   Latest AUVSICameraCode as of 5/31/2013.
   Has four separate threads to:
   1) Wait for image and get from camera
   2) Save image to disk
   3) Wait for telemetry data and save it to txt file
   4) Wait for GPS data and add it to telemetry to be saved

   Code is split across 7 different source files, not including custom front end
   This file initializes and starts all worker threads

   Telemetry listens from /dev/ttyUSB?
   GPS listens from /dev/ttyUSB?
   Arduino pushes to /dev/ttyUSB?

   Works on Atom system

BUG: if quality is changed during a shoot cycle, weird stuff happens. Avoid doing it.
Addendum: If using the proper front-end file creator on the ground, this should never happen
Addendum to Addendum: If it does happen anyway, increase STOP_CYCLE_COUNT in CameraControl.c

TODO: Clearly define and standardize function error codes
TODO: Ability to change other settings (not priority)
TODO: Offload logic from FrontEnd to onboard code
 */

void usage(){
	printf("Valid Usage:\n\n");
	printf("Either 0 or 3 parameters. If 0, will use default ttyUSB port allocations.\n");
	printf("If 3 parameters, will use 3 space delimitedi single digit numbers as ttyUSB allocations.\n");
	printf("Order of parameters is: OpenPilot, Arduino, GPS.\n\n");
}

int initSuite(){
	initImageSync();
	initTelListener();
	initAPMControl();
	initGPSListener();
}

int main(int argc, char ** argv){

	if(argc != 1 && argc != 4){
		usage();
		exit(1);
	}
	if(argc == 4){
		if(strlen(argv[1]) > 1 || strlen(argv[2]) > 1 ||strlen(argv[3]) > 1){
			usage();
			printf("All arguments must be 1 character long.\n\n");
			exit(1);
		}
		if(argv[1][0] < 0x30 || argv[1][0] > 0x39 ||
				argv[2][0] < 0x30 || argv[2][0] > 0x39 ||
				argv[3][0] < 0x30 || argv[3][0] > 0x39){
			usage();
			printf("All arguments must be numbers (0-9).\n\n");
			exit(1);
		}
		if(argv[1][0] == argv[2][0] ||
				argv[2][0] == argv[3][0] ||
				argv[1][0] == argv[3][0]){
			usage();
			printf("Arguments must be unique!\n\n");
			exit(1);
		}
	}

	char OP[15] = "/dev/ttyUSB";
	char APM[15] = "/dev/ttyUSB";
	char GPS[15] = "/dev/ttyUSB";
	if(argc == 4){
		strcat(OP, argv[1]);
		strcat(APM, argv[2]);
		strcat(GPS, argv[3]);
	}
	else{
		strcat(OP, DEFAULT_OP);
		strcat(APM, DEFAULT_APM);
		strcat(GPS, DEFAULT_GPS);
	}
	printf("%s\t%s\t%s\n", OP, APM, GPS);
	setTTYPorts(OP, APM, GPS);

	//Initialize all sub classes
	initSuite();
	
	//Spawning the four worker threads
	pthread_t telemetryThread, GPSThread, getThread, saveThread;

	pthread_create(&getThread, NULL, GetEvents, NULL);
	pthread_create(&saveThread, NULL, SaveFiles, NULL);
	pthread_create(&GPSThread, NULL, GPSListenControl, NULL);
	pthread_create(&telemetryThread, NULL, telemetrySync, NULL);

	pthread_join(getThread, NULL);
	pthread_join(saveThread, NULL);
	pthread_join(GPSThread, NULL);
	pthread_join(telemetryThread, NULL);
}
