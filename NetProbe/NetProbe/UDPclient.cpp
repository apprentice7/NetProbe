//UDPclient for use
//When command line argument is s, this function will be called

#include "stdafx.h"

//Struct to display the statistics
struct Para{
	int PktNum;
	int RfTime;
	double Elapsed;
	unsigned long Pkts;
	float TRate;
	float JNew;
	float JOld;
};

mtx_t Mutexu;

int NetProbeUDPS(char *RefTime, char *Hostname, char *Hostport, char *Packetsize, char* Rate, char* Num){
	//Socket parameters
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);
	
	//Statistics parameters
	char *SendBuf;
	int PktSize = atoi(Packetsize);
	int RfTime = atoi(RefTime);
	int PktNumber = atoi(Num);
	long ContrlRate = atoi(Rate);
	long x = -1; //Sequential number starts from 0
	float Tn = 0;
	float Ti = 0;
	float Told = 0;
	double StdElapsed = 0;
	double SlpTime = 0;

	//Initialize winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Initialization failed. Error code : %d\n", WSAGetLastError);
		exit(EXIT_FAILURE);
	}
	//Create Socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR){
		printf("socket() failed to create. Error code : %d\n", WSAGetLastError);
		exit(EXIT_FAILURE);
	}
	//Setup socket address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_addr.S_un.S_addr = inet_addr(Hostname);
	si_other.sin_port = htons(atoi(Hostport));

	//Initialize a new struct to store data
	struct Para DataS;
	struct Para* PU;
	PU = &DataS;
	DataS.RfTime = RfTime;
	DataS.PktNum = PktNumber;
	DataS.Elapsed = 0;
	DataS.Pkts = 0;
	DataS.TRate = 0;
	DataS.JNew = 0;
	DataS.JOld = 0;
	
	//start the timer
	ES_FlashTimer UDPct = ES_FlashTimer();
	ES_FlashTimer test = ES_FlashTimer();
	UDPct.Start();

	//Intialize a new thread
	thrd_t udp;
	if (thrd_create(&udp, Display4, (void*)PU) == thrd_error){
		printf("Display thread is failed\n");
		exit(EXIT_FAILURE);
	}
	mtx_init(&Mutexu, mtx_plain);

	SendBuf = (char *)malloc(sizeof(char)*PktSize);

	while (1){
		test.Start();
		memset(SendBuf, 1, PktSize);
		x = x + 1;
		memcpy(SendBuf, &x, sizeof(x));		//allocate sequential number

		if (sendto(s, SendBuf, PktSize, 0, (struct sockaddr *)&si_other, slen) == SOCKET_ERROR){
			printf("sendto() failed. Error code : %d\n", WSAGetLastError);
			exit(EXIT_FAILURE);
		}
		else{
			mtx_lock(&Mutexu);
			PU->Elapsed = (double)UDPct.Elapsed() / 1000;  	//Elapsed time
			PU->Pkts = PU->Pkts + 1;				//Packet number sent
			PU->TRate = (long double)PU->Pkts * PktSize /(double)(PU->Elapsed*1024*1024);	//Transmission rate
			//Compute Jitter time
				Tn = (float)PU->Elapsed / PU->Pkts;
				Ti = (float)PU->Elapsed - Told;
				PU->JNew = (float)(PU->JOld * (PU->Pkts - 1) + fabs(Ti - Tn)) / PU->Pkts;		//Jitter to be print out
				PU->JOld = PU->JNew;
				Told = PU->Elapsed;
			//Control rate
			StdElapsed = (long double)PU->Pkts * PktSize / ContrlRate;
			if (StdElapsed > PU->Elapsed){
				SlpTime = 1000*(StdElapsed - PU->Elapsed);
				Sleep(SlpTime);
			}
			mtx_unlock(&Mutexu);
		//Control Packet Number	
		}
		if (PktNumber != 0){
			if (PU->Pkts == PktNumber){
				printf("Transmission completed\n");
				break;
			}
		}
		
	}
	thrd_join(udp, NULL);
	free(SendBuf);
	closesocket(s);
	WSACleanup();
	return 0;
}

int Display4(void* Ptr){
	struct Para* data = (struct Para *)Ptr;
	while (1){
		mtx_lock(&Mutexu);
		printf("\rElapsed[%.1fs] Pkts[%d] Rate[%.2fMbps] Jitter[%.4fms]", data->Elapsed, data->Pkts, data->TRate, (1000 * data->JNew));
		mtx_unlock(&Mutexu);
		Sleep(data->RfTime);
		if (data->PktNum != 0){
			if (data->Pkts == data->PktNum){
				printf("Transmission completed\n");
				break;
			}
		}
	}
	return 0;
}