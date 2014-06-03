#include <stdio.h>
#include <string.h>

#include <gphoto2/gphoto2.h>

Camera* my_Camera; 
GPContext* my_Context;
char TLMTTY[15];

int initCamera(){
    gp_camera_new(&my_Camera);
    my_Context = gp_context_new();

    int result = gp_camera_init(my_Camera, my_Context);
    printf("\n\nCamera Init Result: %d\n\n", result);

    if(result == -105){
        printf("Camera not found, quitting.\n\n");
        return -105;
    } 
    printf("initcamer thingy\n");	

    return 0;
}

Camera* getMyCamera(){
    return my_Camera;
}

GPContext* getMyContext(){
    return my_Context;
}

int setTTYPorts(char* TLM){
    strncpy(TLMTTY, TLM, 15);
    return 0;
}

char* getTLMTTY()   {
    return TLMTTY;
}
