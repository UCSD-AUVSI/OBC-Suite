#include <stdio.h>
#include <gphoto2/gphoto2.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

int main(int argc, char** argv){
    //Initializes Camera object used to represent the Camera we have connected
    Camera* my_Camera; 
	gp_camera_new(&my_Camera);

    //Context object passed around to the methods. Holds some info, we don't
    //really use it except to call functions
	GPContext* my_Context = gp_context_new();
    int result;
    
    //Initializes the Camera (autopopulates any info by autodetecting cameras via USB)
	result = gp_camera_init(my_Camera, my_Context);
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
	for(i = 0; i < gp_list_count(my_List); i++){
		const char *name, *value;
		gp_list_get_name(my_List, i, &name);
		gp_list_get_name(my_List, i, &value);
		printf("\nName: %s\tValue: %s\n\n", name, value);
	}
#endif
        	
    //Pulls and prints the argument given to the program. Supposed to be an int representing number of pictures to take
	int pictureCount = atoi(argv[1]);
	printf("\nPictureCount: %d\n\n", pictureCount);
    
    CameraFile* my_File_Array[pictureCount];
    	
    int i;
    struct timespec start, end;
    CameraFile* my_File;
	for(i = 1; i <= pictureCount; i++){
        // get the time
        clock_gettime(CLOCK_REALTIME, &start);

        //Initializes a CameraFile (holds picture info) object for later use
		result = gp_file_new(&my_File);
//		printf("\nCamera File Create Result: %d\n\n", result);
        
        //Takes a picture, saves to the folder/file location we have specified
		CameraFilePath my_FP;
		result = gp_camera_capture(my_Camera, GP_CAPTURE_IMAGE, &my_FP, my_Context);
//		printf("\nCamera Capture Result: %d\n\n", result);
//      printf("Saved to: %s / %s\n", my_FP.folder, my_FP.name);
        
        //Pulls the picture off the SD card and puts it into the CameraFile object
		result = gp_camera_file_get(my_Camera, my_FP.folder, my_FP.name, GP_FILE_TYPE_NORMAL, my_File, my_Context);
//		printf("\nCamera File Get Result: %d\n\n", result);
		
        my_File_Array[i - 1] = my_File;

        //Frees the file object
        result = gp_camera_file_delete(my_Camera, my_FP.folder, my_FP.name, my_Context);
//        printf("File Delete Result: %d\n", gp_camera_file_delete(my_Camera, my_FP.folder, my_FP.name, my_Context));

        clock_gettime(CLOCK_REALTIME, &end);
        if(end.tv_nsec - start.tv_nsec < 0)
            printf("\tTime elapsed: %d.%ld\n", end.tv_sec - start.tv_sec - 1, 1000000000 + end.tv_nsec - start.tv_nsec);
        else
           printf("\tTime elapsed: %d.%ld\n", end.tv_sec - start.tv_sec, end.tv_nsec - start.tv_nsec);
	}
	
    clock_gettime(CLOCK_REALTIME, &start);
    for(i = 0; i < pictureCount; i++){
        char filename[20];
        sprintf(filename, "IMG_%d.jpg", (i+1));

        //Saves the CameraFile object to disk in the local directory with the given name
		result = gp_file_save(my_File_Array[i], filename);
		printf("File Save Result: %d\n\n", result);
		gp_file_free(my_File_Array[i]);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    if(end.tv_nsec - start.tv_nsec < 0)
        printf("\tDownload time: %d.%ld\n", end.tv_sec - start.tv_sec - 1, 1000000000 + end.tv_nsec - start.tv_nsec);
    else
        printf("\tDownload time: %d.%ld\n", end.tv_sec - start.tv_sec, end.tv_nsec - start.tv_nsec);


    //Frees any info associated with the Camera
	return gp_camera_exit(my_Camera, my_Context);
}
