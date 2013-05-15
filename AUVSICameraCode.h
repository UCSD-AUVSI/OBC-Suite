int setupSeriale();
void * telemetrySync();
void * GetEvents();
void * SaveFiles();
int AddFile();
struct CF* RemoveFile();
int CameraControl();
int CaptureControl(char mode);
int QualityControl(int quality);
