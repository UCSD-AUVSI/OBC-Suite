CC = gcc
LIBS = -lpthread -lgphoto2
CFLAGS = -o
SOURCES = AUVSICameraCode.c ImageSync.c TLMSync.c CameraControl.c SerialControl.c SharedInfo.c
HEADERS= ImageSync.h TLMSync.h CameraControl.h SerialControl.h SharedInfo.h

CameraCode: $(SOURCES) $(HEADERS)
	$(CC) $(SOURCES) $(CFLAGS) $@ $(LIBS)

FrontEnd: Settings.c
	$(CC) Settings.c $(CFLAGS) $@

clean: 
	rm -f ./*.txt ./*.jpg 
