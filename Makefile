CC = gcc
LIBS = -lpthread -lgphoto2
CFLAGS = -o
SOURCES = AUVSICameraCode.c ImageSync.c TelemetrySync.c GPSSync.c CameraControl.c SerialControl.c CameraInfo.c
HEADERS= ImageSync.h TelemetrySync.h GPSSync.h CameraControl.h SerialControl.h CameraInfo.h

CameraCode: $(SOURCES) $(HEADERS)
	$(CC) $(SOURCES) $(CFLAGS) $@ $(LIBS)

FrontEnd: Settings.c
	$(CC) Settings.c $(CFLAGS) $@

clean: 
	rm -f ./*.txt ./*.jpg 
