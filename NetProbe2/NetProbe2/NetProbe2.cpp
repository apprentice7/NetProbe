/*
* File         : NetProbe
* Module       : Main function of NetProbe
* System       : WIN32/Linux
* Project      : IERG4180 project
* Start Date   : 14th March. 2015
* Version      : 01.00
* Version Date : 14-03-2015
* Designer     : Kang Kai
* Programmer   : Kang Kai
* Remarks      : TCP UDP transmission end
* References   :
*
*/

#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")



int main(int argc, char* argv[])
{
	if (argc != 5 && argc != 9){
		printf("Input Argument Error.\n");
		return 0;
	}
	else{
		if (argc == 5 && strcmp(argv[1], "s") == 0){
			NetProbeServer(argv[2], argv[3], argv[4]); //Send Mode
		}
		else if (strcmp(argv[5], "tcp") == 0 || strcmp(argv[5], "TCP") == 0){
			NetProbeClientT(argv[2], argv[3], argv[4], argv[6], argv[7], argv[8]);
		}
		else if (strcmp(argv[5], "udp") == 0 || strcmp(argv[5], "UDP") == 0){
			NetProbeClientU(argv[2], argv[3], argv[4], argv[6], argv[7], argv[8]);
		}
	}
	return 0;
}

