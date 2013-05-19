#include <stdio.h>
#include <string.h>

int isShooting = 0;

void changeSpeed(int* speed){
	int newSpeed = 0;
	char input;
	system("clear");
	do{
		if(input != '\n'){	
			printf("\nEnter new speed (1,2,or 4) in Hz: ");
		}
		scanf("%c", &input);
		newSpeed = (int)input - 0x30;
	}while(newSpeed != 1 && newSpeed != 2 && newSpeed != 4);

	*speed = newSpeed;
}

void createStartFile(int speed){
	char arg[30];
	strcpy(arg, "");
	sprintf(arg, "echo \"%d\" > start.txt", speed);
	system(arg);
}
int main(){
	char choice = 0x29;
	int speed = 1;
	FILE * startFile;

	while(choice != 0x35){
		system("clear");
		printf("(0) Stop shooting\n");
		printf("(1) Start shooting\n");
		printf("(2) Large Fine JPEG\n");
		printf("(3) Medium Fine JPEG\n");
		printf("(4) Change Shoot Speed, Currently %d Hz\n", speed);
		printf("(5) Quit\n");
		printf("\nChoose wisely: ");
		scanf("%c", &choice);

		switch(choice){
			case 0x30:
				system("touch stop.txt"); 
				isShooting = 0;
				break;
			case 0x31: 
				createStartFile(speed);
				isShooting = 1;
				break;
			case 0x32: 
				if(isShooting){
					system("touch stop.txt");
					createStartFile(speed);
				}
				system("touch large.txt"); 
				break;
			case 0x33: 
				if(isShooting){
					system("touch stop.txt");
					createStartFile(speed);
				}
				system("touch medium.txt"); 
				break;
			case 0x34: changeSpeed(&speed); break;
			default: break;
		}
	}
}
