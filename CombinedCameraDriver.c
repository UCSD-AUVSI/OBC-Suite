#include <stdio.h>
#include <gphoto2/gphoto2.h>
#include <string.h>
#include <time.h>

int GetPicture(CameraFile *cf, int wait_for_capture){
	//Initializes Camera object used to represent the Camera we have connected
   	Camera* my_Camera; 
	gp_camera_new(&my_Camera);

   	//Context object passed around to the methods. Holds some info, we don't
   	//really use it except to call functions
	GPContext* my_Context = gp_context_new();
    
   	//Initializes the Camera (autopopulates any info by autodetecting cameras via USB)
	int result = gp_camera_init(my_Camera, my_Context);
	printf("\nCamera Init Result: %d\n\n", result);

#ifdef PRINT_SUMMARY
	//Prints a summary of all the info we know about the camera that's been found
   	CameraText my_Summary = {1};
	result = gp_camera_get_summary(my_Camera, &my_Summary, my_Context);
	printf("\nSummary Result: %d\n\n", result);
	printf("\nSummary: %s\n\n", (&my_Summary)->text);
#endif    

#ifdef PRINT_FOLDERS
    //Lists all the folders in the current folderpath
	CameraList* my_List;
	gp_list_new(&my_List);
	gp_camera_folder_list_folders(my_Camera, folderpath, my_List, my_Context);
	int i;
	for(i = 0; i < gp_list_count(my_List); i++){
		const char *name, *value;
		gp_list_get_name(my_List, i, &name);
		gp_list_get_name(my_List, i, &value);
		printf("\nName: %s\tValue: %s\n\n", name, value);
	}
#endif
	
	struct timespec ts_start, ts_end;

	if(!wait_for_capture){
       	clock_gettime(CLOCK_REALTIME, &ts_start);

       	gp_file_ref(cf);
		 
       	//Takes a picture, saves to the folder/file location we have specified
		result = gp_camera_capture_preview(my_Camera, cf, my_Context);
       	printf("\nCapture result: %d\n", result); 

		clock_gettime(CLOCK_REALTIME, &ts_end);
		if(ts_end.tv_nsec - ts_start.tv_nsec < 0)
			printf("\nTime for Capture: %d.%ld\n", ts_end.tv_sec - ts_start.tv_sec - 1, 1000000000 + ts_end.tv_nsec - ts_start.tv_nsec);
		else
			printf("\nTime for Capture: %d.%ld\n", ts_end.tv_sec - ts_start.tv_sec, ts_end.tv_nsec - ts_start.tv_nsec);
	}
	else{
		void* retevent;
		CameraEventType my_Type;
		while(my_Type != GP_EVENT_FILE_ADDED){
			result = gp_camera_wait_for_event(my_Camera, 100, &my_Type, &retevent, my_Context);
			if(my_Type == GP_EVENT_FILE_ADDED){
				//Start timer for getting this file.
				//WILL NOT INCLUDE TIME TAKEN TO RECOGNIZE NEW FILE CREATION
				clock_gettime(CLOCK_REALTIME, &ts_start);
				
				CameraFilePath* my_FP_ptr = (CameraFilePath*)retevent;
				CameraFilePath my_FP = *my_FP_ptr;
				printf("File added at folder %s, file name is %s.\n", my_FP.folder, my_FP.name);
					 
				//Pulls the picture off the camera and puts it into the CameraFile object
				result = gp_camera_file_get(my_Camera, my_FP.folder, my_FP.name, GP_FILE_TYPE_NORMAL, cf, my_Context);
				printf("Camera File Get Result: %d\n", result);
				
				//Stop timer
				clock_gettime(CLOCK_REALTIME, &ts_end);
				//Print time it took to get file
				if(ts_end.tv_nsec - ts_start.tv_nsec < 0)
					printf("\nTime for Get, Less: %d.%ld\n", ts_end.tv_sec - ts_start.tv_sec - 1, 1000000000000 + ts_end.tv_nsec - ts_start.tv_nsec);
				else
					printf("\nTime for Get: %d.%08ld\n", ts_end.tv_sec - ts_start.tv_sec, ts_end.tv_nsec - ts_start.tv_nsec);
			}    
			else if (my_Type == GP_EVENT_TIMEOUT){
				printf("Timed out\n\n");
			}
			else {
					printf("Other event\n\n");
			}
		}	
	}
	
    return gp_camera_exit(my_Camera, my_Context);
}

int main(int argc, char ** argv){
	CameraFile* my_CF;
	gp_file_new(&my_CF);
	
	int wait_for_capture = atoi(argv[1]);
	
	GetPicture(my_CF, wait_for_capture);

	gp_file_save(my_CF, "Capture.jpg");
}
