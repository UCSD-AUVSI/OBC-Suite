#include <stdio.h>
#include <gphoto2/gphoto2.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv){
    //Initializes Camera object used to represent the Camera we have connected
    Camera* my_Camera; 
	gp_camera_new(&my_Camera);

    //Context object passed around to the methods. Holds some info, we don't
    //really use it except to call functions
	GPContext* my_Context = gp_context_new();
    
    //Initializes the Camera (autopopulates any info by autodetecting cameras via USB)
	int result = gp_camera_init(my_Camera, my_Context);
	printf("\nCamera Init Result: %d\n\n", result);
        	
    //Pulls and prints the argument given to the program. Supposed to be an int representing number of pictures to take
	int pictureCount = atoi(argv[1]);
	printf("\nPictureCount: %d\n\n", pictureCount);
    	
    int i;
	struct timespec ts_overall_b, ts_overall_f, ts_int_b, ts_int_f;
	clock_gettime(CLOCK_REALTIME, &ts_overall_b);
    //CameraFile* my_File;
	CameraFile* my_FileArray[pictureCount];
	for(i = 0; i < pictureCount; i++){
        clock_gettime(CLOCK_REALTIME, &ts_int_b);
		//Initializes a CameraFile (holds picture info) object for later use
		result = gp_file_new(&(my_FileArray[i]));
        gp_file_ref(my_FileArray[i]);
		 
        //Takes a picture, saves to the folder/file location we have specified
		result = gp_camera_capture_preview(my_Camera, my_FileArray[i], my_Context);
       	printf("\nCapture %d result: %d\n", i+1, result); 

		clock_gettime(CLOCK_REALTIME, &ts_int_f);
		if(ts_int_f.tv_nsec - ts_int_b.tv_nsec < 0)
			printf("\nTime for Capture %d: %d.%ld\n", i+1, ts_int_f.tv_sec - ts_int_b.tv_sec - 1, 1000000000 + ts_int_f.tv_nsec - ts_int_b.tv_nsec);
		else
			printf("\nTime for Capture %d: %d.%ld\n", i+1, ts_int_f.tv_sec - ts_int_b.tv_sec, ts_int_f.tv_nsec - ts_int_b.tv_nsec);
	
    }
	clock_gettime(CLOCK_REALTIME, &ts_overall_f);
	if(ts_overall_f.tv_nsec - ts_overall_b.tv_nsec < 0){
		printf("\nTime for Capture %d: %d.%ld\n", i+1, ts_overall_f.tv_sec - ts_overall_b.tv_sec - 1, 1000000000 + ts_overall_f.tv_nsec - ts_overall_b.tv_nsec);
	}
	else{
		printf("\nTime for Capture %d: %d.%ld\n", i+1, ts_overall_f.tv_sec - ts_overall_b.tv_sec, ts_overall_f.tv_nsec - ts_overall_b.tv_nsec);
	}

	for(i = 0; i < pictureCount; i++){
        char filenamenum[20];
        sprintf(filenamenum, "%s%d%s", "capt", i+1, ".jpg");
        printf("File Name: %s\n", filenamenum);
 
        //Saves the CameraFile object to disk in the local directory with the given name
		result = gp_file_save(my_FileArray[i], filenamenum);
		printf("File Save Result: %d\n\n", result);
	}
    //Frees any info associated with the Camera
	return gp_camera_exit(my_Camera, my_Context);
}
