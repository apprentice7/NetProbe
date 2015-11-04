//TCPclient for use
//When the command line argument starts with r, this function will be called

#include "stdafx.h"

//Struct to display the statistics
struct Para{
	int PktNum;
	int RfTime;
	double Elapsed;
	unsigned long Pkts;
	double TRate;
	float JNew;
	float JOld;
};

mtx_t Mutext;

int NetProbeTCPS(char *RefTime, char *Hostname, char *Hostport, char *Packetsize, char* Rate, char* Num){
	//Socket parameters
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	int send_len;

	//Statistics parameters
	char *SendBuf;
	int PktSize = atoi(Packetsize);
	int RfTime = atoi(RefTime);
	int PktNumber = atoi(Num);
	long ContrlRate = atoi(Rate);
	unsigned long TotalLen = 0;		//Total bytes transmitted
	float Tn = 0;
	float Ti = 0;
	double Told = 0;
	double StdElapsed = 0;
	double SlpTime = 0;

	//Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed to Initialize, Error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//Create socket
	if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET){
		printf("Could not create socket : d%", WSAGetLastError());
	}
	//Prepare sockaddr_in address structure
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(Hostname);
	server.sin_port = htons(atoi(Hostport));
	//Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Connect error\n");
		exit(EXIT_FAILURE);
	}

	//Initialize the struct
	struct Para DataS;
	struct Para* PT;
	PT = &DataS;
	DataS.RfTime = RfTime;
	DataS.PktNum = PktNumber;
	DataS.Elapsed = 0;
	DataS.Pkts = 0;
	DataS.TRate = 0;
	DataS.JNew = 0;
	DataS.JOld = 0;

	//start the timer
	ES_FlashTimer TCPct = ES_FlashTimer();
	ES_FlashTimer test = ES_FlashTimer();
	TCPct.Start();

	//Intialize a new thread
	thrd_t tcp;
	if (thrd_create(&tcp, Display3, (void*)PT) == thrd_error){
	printf("Display thread is failed\n");
	exit(EXIT_FAILURE);
	}
	mtx_init(&Mutext, mtx_plain);
	
	SendBuf = (char *)malloc(sizeof(char)*PktSize);

	while (1){
		test.Start();
		memset(SendBuf, 1, sizeof(char)*PktSize);
		if ((send_len = send(s, SendBuf, PktSize, 0)) == SOCKET_ERROR){
			printf("send() failed with error code: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		else{
			mtx_lock(&Mutext);
			PT->Elapsed = (double)TCPct.Elapsed() / 1000;		//Elapsed time in second
			PT->Pkts = PT->Pkts + 1;				//Packet number sent
			PT->TRate = (double)PT->Pkts * PktSize / (double)(PT->Elapsed * 1024 * 1024);	//Transmission rate
			//Compute Jitter time 
				Tn = (float)PT->Elapsed / PT->Pkts;
				Ti = (float)PT->Elapsed - Told;
				PT->JNew = (double)(PT->JOld * (PT->Pkts - 1) + (double)fabs(Ti - Tn)) / PT->Pkts;		//Jitter to be printed out
				PT->JOld = PT->JNew;
				Told = PT->Elapsed;
			//Control Rate
			StdElapsed = (long double)PT->Pkts * PktSize / ContrlRate;
			if (StdElapsed > PT->Elapsed){
				SlpTime = 1000 * (StdElapsed - PT->Elapsed);
				Sleep(SlpTime);
			}
			mtx_unlock(&Mutext);
			//Control Packet Number
			}
		if (PktNumber != 0){
			if (PT->Pkts == PktNumber){
				printf("Transmission completed\n");
				break;
			}
		}
	}
	thrd_join(tcp,NULL);
	free(SendBuf);
	closesocket(s);
	WSACleanup();
	return 0;
}
int Display3(void* Ptr){
	struct Para* data = (struct Para *)Ptr;
	while (1){
		mtx_lock(&Mutext);
		printf("\rElapsed[%.1fs] Pkts[%ld] Rate[%.2fMbps] Jitter[%.4fms]", data->Elapsed, data->Pkts, data->TRate, (1000 * data->JNew));
		mtx_unlock(&Mutext);
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