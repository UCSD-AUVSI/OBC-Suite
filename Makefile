CC = gcc
LIBS = -lpthread -lgphoto2

AUVSICameraCode: AUVSICameraCode.c 
	$(CC) AUVSICameraCode.c AUVSICameraCode.h -o CameraCode $(LIBS)

Settings: Settings.c
	$(CC) Settings.c -o xSettings

Clean: 
	rm *.txt *.jpg
