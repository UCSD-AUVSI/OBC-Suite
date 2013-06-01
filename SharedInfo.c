#include <stdio.h>
#include <string.h>

#include <gphoto2/gphoto2.h>

Camera* my_Camera; 
GPContext* my_Context;
char OPTTY[15], APMTTY[15], GPSTTY[15];

int initCamera(){
	gp_camera_new(&my_Camera);
	my_Context = gp_context_new();

	int result = gp_camera_init(my_Camera, my_Context);
	printf("\n\nCamera Init Result: %d\n\n", result);

	if(result == -105){
		printf("Camera not found, quitting.\n\n");
		return -105;
	} 
	
	return 0;
}

Camera* getMyCamera(){
	return my_Camera;
}

GPContext* getMyContext(){
	return my_Context;
}

int setTTYPorts(char* OP, char* APM, char* GPS){
	strncpy(OPTTY, OP, 15);
	strncpy(APMTTY, APM, 15);
	strncpy(GPSTTY, GPS, 15);
	return 0;
}

char* getOPTTY(){
	return OPTTY;	
}

char* getAPMTTY(){
	return APMTTY;
}

char* getGPSTTY(){
	return GPSTTY;
}
