/*
* File         : NetProbe
* Module       : Main function of NetProbe
* System       : WIN32
* Project      : IERG4180 project
* Start Date   : 15th Feb. 2015
* Version      : 01.00
* Version Date : 15-02-2015
* Designer     : Kang Kai
* Programmer   : Kang Kai
* Remarks      : TCP UDP transmission end
* References   :
*
*/

#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
int main(int argc, char **argv){

	//Determine the mode of NetProbe
	if (strcmp(argv[1], "s") == 0 || strcmp(argv[1], "S") == 0){
		if (argc != 9){
			printf("Argument Error\n");
			printf("NetProbe s [refresh_interval] [remote_host] [remote_port] [protocol [packet_size] [rate] [num]\n");
		}
		else{
			//Determine the protocal of transmission 
			if (strcmp(argv[5], "udp") == 0 || strcmp(argv[5], "UDP") == 0){
				NetProbeUDPS(argv[2], argv[3], argv[4], argv[6], argv[7], argv[8]);
			}
			else
				NetProbeTCPS(argv[2], argv[3], argv[4], argv[6], argv[7], argv[8]);
		}
	}
	//Determine the mode of NetProbe
	else if (strcmp(argv[1], "r") == 0 || strcmp(argv[1], "R") == 0){
		if (argc != 7){
			printf("Argument Error\n");
			printf("NetProbe r [refresh_interval] [local_host] [local_port] [protocol] [packet_size]\n");
		}
		else{
			//Determine the protocal of transmission 
			if (strcmp(argv[5], "udp") == 0 || strcmp(argv[5], "UDP") == 0){
				NetProbeUDPR(argv[2], argv[3], argv[4], argv[6]);
			}
			else
				NetProbeTCPR(argv[2], argv[3], argv[4], argv[6]);
		}
	}

	//Determine the mode of NetProbe
	else if (strcmp(argv[1], "h") == 0 || strcmp(argv[1], "H") == 0){
		if (argc != 3){
			printf("Argument Error\n");
			printf("NetProbe h [hostname]\n");
		}
		else{
			NetProbeH(argv[2]);
		}
	}

	//Claim errror of command line argument
	else{
		printf("Second command line argument should be 'r' 's' or 'h'\n");
	}
}