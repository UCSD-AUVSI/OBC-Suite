#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <gphoto2/gphoto2.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "SharedInfo.h"

struct termios options;

static int currTLM = -1;

int openTLMSerial(int* desc){
    *desc = open(getTLMTTY(), O_RDWR | O_NOCTTY);
    if(*desc == -1){
        return -1;
    }
    else{
        tcgetattr(*desc, &options);
        cfsetispeed(&options, B57600);
        cfsetospeed(&options, B57600);
        options.c_cflag |= (CLOCAL |CREAD);
        tcsetattr(*desc, TCSANOW, &options);
        return 0;
    }
}

int getTLMSerial(int *desc){
    if(currTLM != -1){ // If the TLM port has already been opened
        *desc = currTLM;
        return 0;
    }
    else{ // TLM port needs to be opened
        if(!openTLMSerial(&currTLM)){ //openTLM succeeded
            *desc = currTLM;
            return 0;
        }
        else { //openTLM failed
            *desc = -1;
            return -1;
        }
    }
}

