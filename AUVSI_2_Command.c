#include <stdio.h>
#include <gphoto2/gphoto2.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
 	Camera* my_Camera;
	gp_camera_new(&my_Camera);
	GPContext* my_Context = gp_context_new();

	int result = gp_camera_init(my_Camera, my_Context);
	printf("\nCamera Init Result: %d\n\n", result);

	char* folderpath = "/store_00010001/DCIM/100CANON";
	int i, j;

/*	CameraText my_Summary = {1};
	result = gp_camera_get_summary(my_Camera, &my_Summary, my_Context);
	printf("\nSummary Result: %d\n\n", result);
	printf("\nSummary: %s\n\n", (&my_Summary)->text);

	CameraList* my_List;
	gp_list_new(&my_List);
	gp_camera_folder_list_folders(my_Camera, folderpath, my_List, my_Context);
	
	for(i = 0; i < gp_list_count(my_List); i++){
		const char *name, *value;
		gp_list_get_name(my_List, i, &name);
		gp_list_get_name(my_List, i, &value);
		printf("\nName: %s\tValue: %s\n\n", name, value);
	}
*/	
	int pictureCount = atoi(argv[1]);
	printf("\nPictureCount: %d\n\n", pictureCount);
	
	for(i = 1; i <= pictureCount; i++){
		char filename[20];
        
        sprintf(filename, "%s%04d%s", "IMG_", i, ".JPG");
		
		printf("\nFilename:%s\n\n", filename);

		CameraFilePath my_FP = {filename, folderpath};
		result = gp_camera_capture(my_Camera, GP_CAPTURE_IMAGE, &my_FP, my_Context);
		printf("\nCamera Capture Result: %d\n\n", result);

		CameraFile* my_File;
		result = gp_file_new(&my_File);
		printf("\nCamera File Create Result: %d\n\n", result);
	
		result = gp_camera_file_get(my_Camera, folderpath, filename, GP_FILE_TYPE_NORMAL, my_File, my_Context);
		printf("\nCamera File Get Result: %d\n\n", result);
	
		const char *data = "";
		unsigned long int size = 0;
		result = gp_file_get_data_and_size(my_File, &data, &size);

		printf("\nFile Info Result: %d\n", result);
		printf("File Info Data: %s\t Size: %ld\n\n", data, size);
				
		result = gp_file_save(my_File, filename);
		printf("\nFile Save Result: %d\n\n", result);

		gp_file_free(my_File);
	}
	
	return gp_camera_exit(my_Camera, my_Context);
}
