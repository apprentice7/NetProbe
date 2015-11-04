/*
* File         : NetProbe
* Module       : Main function of NetProbe
* System       : WIN32/Linux
* Project      : IERG4180 project
* Start Date   : 19th April 2015
* Version      : 01.00
* Version Date : 19-04-2015
* Designer     : Kang Kai
* Programmer   : Kang Kai
* Remarks      : Main entrance
* References   :
*
*/

//#include "stdafx.h"
#include "Header.h"
#pragma comment(lib, "ws2_32.lib")



int main(int argc, char* argv[]){
	if (argc != 4 && argc != 5){
		printf("=====Wrong input parameters, see instructions below=====.\n");
		printf("./NetProbe s [port] [Directory]\n");
		printf("s: server mode\n");
		printf("port: the port for server.\n");
		printf("Directory: the directory to be synchronized\n");
		printf("\n");
		printf("./NetProbe c [IP address] [port] [Directory]\n");
		printf("c: client mode\n");
		printf("IP address: server IP address\n");
		printf("port: the port for client\n");
		printf("Directory: the directory to be synchronized\n");
		return 0;
	}
	else if (argc == 4 && strcmp(argv[1],"s") == 0){
		printf("server mode...\n");
		Server(argv[2], argv[3]);
	}
	else if (argc == 5 && strcmp(argv[1],"c") == 0){
		printf("client mode..\n");
		Client(argv[2], argv[3],argv[4]);
	}
	else{
		printf("=====Wrong input parameters, see instructions below=====.\n");
		printf("./NetProbe s [port]\n");
		printf("s: server mode\n");
		printf("port: the port for server.\n");
		printf("\n");
		printf("./NetProbe c [IP address] [port] [Directory]\n");
		printf("c: client mode\n");
		printf("IP address: server IP address\n");
		printf("port: the port for client\n");
		printf("Directory: the directory to be synchronized\n");
		return 0;
	}
	return 0;

}

