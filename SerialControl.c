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
	int read_len, flag;
	int baudrate;
	int mcs;
	fd_set readfds;
	char buf[64];
	struct termios tty, old;

	*desc = open(getGPSTTY(), O_RDWR | O_NOCTTY | O_NDELAY);

	if(*desc < 0)
		return -1;

	flag = fcntl(*desc, F_GETFL, 0);
	fcntl(*desc, F_SETFL, flag & ~O_NDELAY);

	//set baudrate to 0 and go back to normal
	printf("set baudrate to 0......\n");
	tcgetattr(*desc, &tty);
	tcgetattr(*desc, &old);
	cfsetospeed(&tty, B0);
	cfsetispeed(&tty, B0);
	tcsetattr(*desc, TCSANOW, &tty);
	tcsetattr(*desc, TCSANOW, &old);
	printf("baudrate is back to normal......\n");

	tcgetattr(*desc, &tty);

	baudrate = B38400;
	cfsetospeed(&tty, baudrate);
	cfsetispeed(&tty, baudrate);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	
	//set into raw, no echo mode
	tty.c_iflag = IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cflag |= CLOCAL | CREAD;

	tty.c_cflag &= ~CRTSCTS;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 5;

	//turn off software control
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	//no parity
	tty.c_cflag &= ~(PARENB | PARODD);

	//1 stopbit
	tty.c_cflag &= ~CSTOPB;

	tcsetattr(*desc, TCSANOW, &tty);

	return 0;
}
