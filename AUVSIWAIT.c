#include <stdio.h>
#include <gphoto2/gphoto2.h>
#include <string.h>
#include <time.h>

int main(int argc, char ** argv){
    //Initializes Camera object used to represent the Camera we have connected
    Camera* my_Camera; 
	gp_camera_new(&my_Camera);

    //Context object passed around to the methods. Holds some info, we don't
    //really use it except to call functions
	GPContext* my_Context = gp_context_new();
    
    //Initializes the Camera (autopopulates any info by autodetecting cameras via USB)
	int result = gp_camera_init(my_Camera, my_Context);
	printf("\n\nCamera Init Result: %d\n\n", result);
   	
	if(result == -105){
		printf("Camera not found, quitting.\n\n");
		return -105;
	} 
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
    
    CameraFile* my_File;
    CameraEventType my_Type;
    void* retevent;
	struct timespec ts_start, ts_end;
	char PrintWaiting = 0;
	while(1){
        result = gp_camera_wait_for_event(my_Camera, 100, &my_Type, &retevent, my_Context);
        if(my_Type == GP_EVENT_FILE_ADDED){
        	//Start timer for getting this file.
			//WILL NOT INCLUDE TIME TAKEN TO RECOGNIZE NEW FILE CREATION
			clock_gettime(CLOCK_REALTIME, &ts_start);
        		
			//Initializes a CameraFile (holds picture info) object for later use
			result = gp_file_new(&my_File);
                   
			CameraFilePath* my_FP_ptr = (CameraFilePath*)retevent;
			CameraFilePath my_FP = *my_FP_ptr;
			printf("\nFile added at folder %s, file name is %s.\n", my_FP.folder, my_FP.name);
             
			//Pulls the picture off the camera and puts it into the CameraFile object
			result = gp_camera_file_get(my_Camera, my_FP.folder, my_FP.name, GP_FILE_TYPE_NORMAL, my_File, my_Context);
			printf("Camera File Get Result: %d\n", result);
            
			//Stop timer
			clock_gettime(CLOCK_REALTIME, &ts_end);
			//Print time it took to get file
			if( (ts_end.tv_nsec - ts_start.tv_nsec) < 0)
				printf("Time for Get: %d.%ld\n", ts_end.tv_sec - ts_start.tv_sec - 1, 1000000000 + ts_end.tv_nsec - ts_start.tv_nsec);
			else
				printf("Time for Get: %d.%08ld\n", ts_end.tv_sec - ts_start.tv_sec, ts_end.tv_nsec - ts_start.tv_nsec);
            
        	//Start timer for saving this file.
			//WILL NOT INCLUDE TIME TAKEN TO RECOGNIZE NEW FILE CREATION
			clock_gettime(CLOCK_REALTIME, &ts_start);
		  			
			//Saves the CameraFile object to disk in the local  with the given name
            result = gp_file_save(my_File, my_FP.name);
            printf("File Save Result: %d\n", result);
            gp_file_free(my_File);

			//Stop timer
			clock_gettime(CLOCK_REALTIME, &ts_end);
			//Print time it took to save file
			if(ts_end.tv_nsec - ts_start.tv_nsec < 0)
				printf("Time for Save: %d.%ld\n\n", ts_end.tv_sec - ts_start.tv_sec - 1, 1000000000 + ts_end.tv_nsec - ts_start.tv_nsec);
			else
				printf("Time for Save: %d.%08ld\n\n", ts_end.tv_sec - ts_start.tv_sec, ts_end.tv_nsec - ts_start.tv_nsec);
        	
			PrintWaiting = 0;
		}
        else {
			if(!PrintWaiting){
            	printf("Waiting for capture...\n");
				PrintWaiting = 1;
			}
        }
    }
    return gp_camera_exit(my_Camera, my_Context);
}
