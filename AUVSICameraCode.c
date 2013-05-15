#include <pthread.h>

#include "TelemetrySync.h"
#include "CameraControl.h"
#include "GPSSync.h"
#include "ImageSync.h"

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

int initSuite(){
	initImageSync();
	initTelListener();
	initAPMControl();
	initGPSListener();
}

int main(int argc, char ** argv){
	
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
