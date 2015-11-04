// UDPserver for use
// When the command line argument starts with r, this function will be called
#include "stdafx.h"

//Struct to display statistics
struct Para{
	int RfTime;
	double Elapsed;
	unsigned long Pkts;
	unsigned long PktLost;
	double LostPtg;
	float TrRate;
	float JtrNew;
	float JtrOld;
};

mtx_t MutexU;

int NetProbeUDPR(char *RefTime, char *Hostname, char *Hostport, char *Packetsize){
	//Socket parameters
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	slen = sizeof(si_other);

	//Statistics parameters
	char *ReceiveBuf;
	int PktSize = atoi(Packetsize);
	int RfTime = atoi(RefTime);
	int Sqn = 0;	//Sequential number
	float Tn = 0;
	float Ti = 0;
	float Told = 0;

	//Initialize winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed to Initialize, Error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		printf("Could not create socket : d%\n", WSAGetLastError());
	}
	//Setup sockaddr_in structure
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(Hostname);
	server.sin_port = htons(atoi(Hostport));
	//Bind the socket
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Bind failed with error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//Intialize a new struct to store data
	struct Para DataS;
	struct Para* PU;
	PU = &DataS;
	DataS.RfTime = RfTime;
	DataS.Elapsed = 0;
	DataS.Pkts = 0;
	DataS.PktLost = 0;
	DataS.LostPtg = 0;
	DataS.TrRate = 0;
	DataS.JtrNew = 0;
	DataS.JtrOld = 0;
	
	//Start the timer
	ES_FlashTimer UDPst = ES_FlashTimer();
	UDPst.Start();

	//Intialize a new thread
	thrd_t UDP;
	if (thrd_create(&UDP, Display1, (void*)PU) == thrd_error){
		printf("Display thread is failed\n");
		exit(EXIT_FAILURE);
	}
	mtx_init(&MutexU, mtx_plain);
	
	ReceiveBuf = (char *)malloc(sizeof(char)*PktSize);
	
	while (1){
		memset(ReceiveBuf, 1, PktSize);

		if ((recv_len = recvfrom(s, ReceiveBuf, PktSize+8, 0, (struct sockaddr *)&si_other, &slen)) == SOCKET_ERROR){
			printf("recvfrom() failed with error code : %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		else{
			mtx_lock(&MutexU);
			PU->Elapsed = (double)UDPst.Elapsed() / 1000;		//Elapsed time
			PU->TrRate = (long double)PU->Pkts * PktSize / (PU->Elapsed*1024*1024);  //Transmission rate
			memcpy(&Sqn, ReceiveBuf, sizeof(ReceiveBuf));				//Sequential number received
			PU->PktLost = Sqn - PU->Pkts;				//Lost packets
			PU->LostPtg = (long double)PU->PktLost /(double)Sqn * 100;		//Lost percentage of packet
			PU->Pkts = PU->Pkts + 1;	//Packet number received
			//Compute Jitter time 
				Tn = (float)PU->Elapsed / PU->Pkts;
				Ti = (float)PU->Elapsed - Told;
				PU->JtrNew = (float)(PU->JtrOld * (PU->Pkts - 1) + fabs(Ti - Tn)) / PU->Pkts;		//Jitter to be printed out
				PU->JtrOld = PU->JtrNew;
				Told = PU->Elapsed;
			mtx_unlock(&MutexU);
		}
	}
	thrd_join(UDP, NULL);
	free(ReceiveBuf);
	closesocket(s);
	WSACleanup();
	return 0;
}

int Display1(void* Ptr){
	struct Para* data = (struct Para *)Ptr;
	while (1){
		mtx_lock(&MutexU);
		printf("\rElapsed[%.1fs] Pkts[%d] Lost[%d, %.2f%%] Rate[%.2fMbps%] Jitter[%.4fms]", data->Elapsed, data->Pkts, data->PktLost, data->LostPtg, data->TrRate, (1000 * data->JtrNew));
		mtx_unlock(&MutexU);
		Sleep(data->RfTime);
	}
	return 0;
}