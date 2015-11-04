//UDPclient for use
//When command line argument is s, this function will be called

#include "stdafx.h"

struct Para{
	unsigned long Pkts;
	int PktLimit;
	long PktLost;
	double LostPtg;
	int RfTime;
	double Elapsed;
	double TRate;
};

mtx_t MutexU;

int NetProbeClientU(char *RefTime, char *ServerHost, char *ServerPort, char* PacketSize, char* Rate, char* Num){

	//Initialize socket parameters
	SOCKET SendSocket;
	struct sockaddr_in server, si_other;
	socklen_t slen = sizeof(si_other);
	int recv_len;

	//Statistics parameters
	char *SendBuf;
	char *ReceiveBuf;
	int RfTime = atoi(RefTime);
	int PktSize = atoi(PacketSize);
	int ContrlRate = atoi(Rate);
	int ContrlPkt = atoi(Num);
	int Sqn = 0;
	ReceiveBuf = (char *)malloc(sizeof(char)*PktSize);
	SendBuf = (char*)malloc(sizeof(char)*PktSize);

	//Initialize a new struct to store data
	struct Para DataS;
	struct Para* PU;
	PU = &DataS;
	DataS.PktLimit = ContrlPkt;
	DataS.RfTime = RfTime;
	DataS.Pkts = 0;
	DataS.Elapsed = 0;
	DataS.TRate = 0;
	DataS.PktLost = 0;
	DataS.LostPtg = 0;

	//Intialize a new thread
	thrd_t udp;
	if (thrd_create(&udp, UClientDisp, (void*)PU) == thrd_error){
		printf("Display thread is failed\n");
		exit(EXIT_FAILURE);
	}
	mtx_init(&MutexU, mtx_plain);

	//Initialize winsock
#ifdef WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Initialization failed. Error code : %d\n", WSAGetLastError);
		exit(EXIT_FAILURE);
	}
#endif
	if ((SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR){
		printf("socket() failed to create. Error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_addr.s_addr = inet_addr(ServerHost);
	si_other.sin_port = htons(atoi(ServerPort));

	//Send parameters to server through buffer
	memcpy(SendBuf, &PktSize, sizeof(int));
	memcpy((SendBuf + 4), &ContrlRate, sizeof(int));
	memcpy((SendBuf + 8), &ContrlPkt, sizeof(int));

	if (sendto(SendSocket, SendBuf, PktSize, 0, (struct sockaddr *)&si_other, slen) == SOCKET_ERROR){
		printf("sendto() failed. Error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//start the timer
	ES_FlashTimer UDPct = ES_FlashTimer();
	//ES_FlashTimer test = ES_FlashTimer();
	UDPct.Start();


	while (1){
		//test.Start();
		memset(ReceiveBuf, '\0', PktSize);

		if ((recv_len = recvfrom(SendSocket, ReceiveBuf, PktSize + 8, 0, (struct sockaddr *)&si_other, &slen)) == SOCKET_ERROR){
			printf("recvfrom() failed with error code : %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		else{
			mtx_lock(&MutexU);
			PU->Elapsed = (double)UDPct.Elapsed() / 1000;  	//Elapsed time
			PU->Pkts = PU->Pkts + 1;				//Packet number sent
			PU->TRate = (long double)PU->Pkts * PktSize / (double)(PU->Elapsed * 1024 * 1024);	//Transmission rate
			memcpy(&Sqn, ReceiveBuf, sizeof(int));
			PU->PktLost = Sqn - PU->Pkts;
			PU->LostPtg = (long double)PU->PktLost / (double)Sqn * 100;
			mtx_unlock(&MutexU);
		}

	}
	thrd_join(udp, NULL);
	free(SendBuf);
	closesocket(SendSocket);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}

int UClientDisp(void* Ptr){
	struct Para* data = (struct Para *)Ptr;
	while (1){
		mtx_lock(&MutexU);
		printf("\rElapsed[%.1fs] Pkts[%d] Lost[%d, %.2f%%] Rate[%.2fMbps]", data->Elapsed, data->Pkts, data->PktLost, data->LostPtg, data->TRate);
		fflush(stdout);
		mtx_unlock(&MutexU);
		Sleep(data->RfTime);
		if (data->PktLimit != 0){
			if (data->Pkts == data->PktLimit){
				printf("Transmission completed\n");
				break;
			}
		}
	}
	return 0;
}
