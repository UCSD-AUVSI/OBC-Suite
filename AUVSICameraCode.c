#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <gphoto2/gphoto2.h>

#include "TLMSync.h"
#include "CameraControl.h"
#include "GPSSync.h"
#include "ImageSync.h"
#include "SharedInfo.h"

#define DEFAULT_TLM "0"

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
    initTLMListener();
    printf("Initializing Listeners/Syncs\n");
    return 0;
}

int main(int argc, char ** argv){

    if(argc != 1 && argc != 2){
        usage();
        exit(1);
    }
    if(argc == 2){
        if(strlen(argv[1]) > 1 ){
            usage();
            printf("All arguments must be 1 character long.\n\n");
            exit(1);
        }
        if(argv[1][0] < 0x30 ){
            usage();
            printf("All arguments must be numbers (0-9).\n\n");
            exit(1);
        }
        if(0){
            usage();
            printf("Arguments must be unique!\n\n");
            exit(1);
        }
    }

    char TLM[15] = "/dev/ttyACM";
    if(argc == 2){
        strcat(TLM, argv[1]);
    }
    else{
        strcat(TLM, DEFAULT_TLM);
    }
    printf("Getting TLM off %s\n", TLM);
    setTTYPorts(TLM);

    //Initialize all sub classes
    initSuite();

    //Spawning the three worker threads
    pthread_t TLMThread, getThread, saveThread;

    pthread_create(&getThread, NULL, GetEvents, NULL);
    printf("Starting Get Thread\n");
    pthread_create(&saveThread, NULL, SaveFiles, NULL);
    printf("Starting Save Thread\n");
    pthread_create(&TLMThread, NULL, TLMSync, NULL);
    printf("Starting TLM Thread\n");

    pthread_join(getThread, NULL);
    pthread_join(saveThread, NULL);
    pthread_join(TLMThread, NULL);
}
