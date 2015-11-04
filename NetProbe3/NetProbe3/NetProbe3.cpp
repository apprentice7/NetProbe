/*
* File         : NetProbe
* Module       : Main function of NetProbe
* System       : WIN32/Linux
* Project      : IERG4180 project
* Start Date   : 29th March. 2015
* Version      : 01.00
* Version Date : 29-03-2015
* Designer     : Kang Kai
* Programmer   : Kang Kai
* Remarks      : Main entrance
* References   :
*
*/

#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")



int main(int argc, char* argv[])
{
	if (argc != 4 && argc != 5){
		printf("=====Wrong input parameters, see instructions below=====.\n");
		printf("./netProbe s [port] [threadmode] [threadnum]\n");
		printf("port: the port for http server.\n");
		printf("threadmode: 'o' means on-demand thread creation mode.\n");
		printf("threadmode: 'p' means thread-pool model, the number of pool should be specified by threadnum.\n");
		printf("threadnum: the number of threads only for thread-pool mode.\n");
		return 0;
	}
	else{
		if (argc == 4 && strcmp(argv[3], "o") == 0){
			printf("On-demand thread creation mode\n");
			printf("Waiting for client to connect...\n");
			NetProbeServerOD(argv[2]); 
		}
		else if (argc == 5 && strcmp(argv[3], "p") == 0){
			printf("Thread-pool mode\n");
			NetProbeServerTP(argv[2], argv[4]);
		}
		else{
			printf("=====Wrong input parameters, see instructions below=====.\n");
			printf("./netProbe s [port] [threadmode] [threadnum]\n");
			printf("port: the port for http server.\n");
			printf("threadmode: 'o' means on-demand thread creation mode.\n");
			printf("threadmode: 'p' means thread-pool model, the number of pool should be specified by threadnum.\n");
			printf("threadnum: the number of threads only for thread-pool mode.\n");
			return 0;
		}
	}
	return 0;
}

