#include <stdio.h>
#include <gphoto2/gphoto2.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "SharedInfo.h"

struct termios options;

int openOPSerial(int* desc){
	*desc = open(getOPTTY(), O_RDONLY | O_NOCTTY);
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

int openAPMSerial(int* desc){
	*desc = open(getAPMTTY(), O_WRONLY | O_NOCTTY);
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

int openGPSSerial(int* desc){
	*desc = open(getGPSTTY(), O_RDONLY | O_NOCTTY);
	if(*desc == -1){
		return -1;
	}
	else{
		tcgetattr(*desc, &options);
		cfsetispeed(&options, B38400);
		cfsetospeed(&options, B38400);
		options.c_cflag |= (CLOCAL |CREAD);
		tcsetattr(*desc, TCSANOW, &options);
		return 0;
	}
}
