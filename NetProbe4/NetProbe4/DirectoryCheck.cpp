//Check the directory specified by user
//exsits: report exist and to be synchronized
//not: create a new directory

#include "Header.h"

void DirCheck(char * Directory){

	struct stat info;
	char * DominDir = (char*)malloc(sizeof(char)* 50);

	strcpy(DominDir, Directory);
	int Len = strlen(Directory);
	if (DominDir[Len - 1]== '\\' ){
		DominDir[Len - 1] = '\0';
	}
	if (stat(DominDir, &info) == 0){
		strcpy(DominDir + strlen(DominDir), "/cloud");
		if (stat(DominDir, &info) == 0){
			if (info.st_mode & S_IFDIR){
				printf("%s is going to be synchronized\n", DominDir);
			}
			else{
				mkdir(DominDir);
				printf("%s is going to be synchronized\n", DominDir);
			}
		}
		else{
			mkdir(DominDir);
			printf("%s is going to be synchronized\n", DominDir);
		}
	}
	else{
		mkdir(DominDir);
		strcpy(DominDir + strlen(DominDir), "/cloud");
		mkdir(DominDir);
		printf("%s is created for synchronizaiton\n", DominDir);
	}
}
	