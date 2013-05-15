#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gphoto2/gphoto2.h>

#include "SerialControl.h"
#include "CameraInfo.h"

#define STOP_CYCLE_COUNT 20

int serialFDAPM;

static int DLWaitCounter = 0;
static int currentMode = -1;
static int currentQuality = -1;

int initAPMControl(){
	if(openAPMSerial(&serialFDAPM) == -1)
	{
		perror("Error setting up APM Serial Port");
		return -1;
	}
	return 0;
}

int getWaitCounter(){
	return DLWaitCounter;
}

int decrementWaitCounter(){
	DLWaitCounter--;
	return DLWaitCounter;
}

void setWaitCounter(int input){
	DLWaitCounter = input;
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

	if(gp_camera_get_config(getMyCamera(), &widget, getMyContext()) != 0){ //Get our current config
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

	if(gp_camera_set_config(getMyCamera(), child, getMyContext()) != 0){ //Set the camera's config to our new, modified config
		free(widget);
		free(child);
		return -1;
	}

	free(widget);
	free(child);

	currentQuality = quality; //Remember what quality we currently have
	return 0;
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
