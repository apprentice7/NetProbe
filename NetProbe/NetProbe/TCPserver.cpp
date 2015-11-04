//TCPserver for use
//When the command line argument starts with r, this function will be called

#include "stdafx.h"

struct Para{
	long PktLost;
	double LostPtg;
	int RfTime;
	float Elapsed;
	unsigned long Pkts;
	double TRate;
	float JNew;
	float JOld;
};

mtx_t MutexT;

int NetProbeTCPR(char *RefTime, char *Hostname, char *Hostport, char *Packetsize){
	//Initialize socket parameters
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c, recv_len;
	c = sizeof(struct sockaddr_in);

	//Statistics parameters
	char *ReceiveBuf;
	int PktSize = atoi(Packetsize);
	int RfTime = atoi(RefTime);
	unsigned long long TotalLen = 0;
	int n = 0;		//To identify times of transmission
	float Tn = 0;
	float Ti = 0;
	float Told = 0;

	//Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed to Initialize, Error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//Create socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
		printf("Could not create socket : d%", WSAGetLastError());
	}
	//Prepare the sockaddr_in structure
	memset(&server, 0, sizeof(struct sockaddr_in));
	memset(&client, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(Hostname);
	server.sin_port = htons(atoi(Hostport));
	//Bind the socket
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//Listern to incoming connections
	listen(s, 10);

	new_socket = accept(s, (struct sockaddr *)&client, &c);

	//Intialize a new struct to store data
	struct Para DataT;
	struct Para* PT;
	PT = &DataT;
	DataT.RfTime = RfTime;
	DataT.Elapsed = 0;
	DataT.Pkts = 0;
	DataT.PktLost = 0;
	DataT.LostPtg = 0;
	DataT.TRate = 0;
	DataT.JNew = 0;
	DataT.JOld = 0;

	//start the timer
	ES_FlashTimer TCPst = ES_FlashTimer();
	TCPst.Start();

	//Intialize a new thread
	thrd_t TCP;
	if (thrd_create(&TCP, Display2, (void*)PT) == thrd_error){
		printf("Display thread is failed\n");
		exit(EXIT_FAILURE);
	}
	mtx_init(&MutexT, mtx_plain);

	ReceiveBuf = (char *)malloc(sizeof(char)*PktSize);

	while (1){
		memset(ReceiveBuf, 0, PktSize);
		if (new_socket == INVALID_SOCKET){
			printf("accept failed with error code: %d\n", WSAGetLastError());
		}
		else{
			if((recv_len = recv(new_socket, ReceiveBuf, PktSize, 0)) == SOCKET_ERROR){
				printf("recv() failed with error code: %d\n", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			else{
				mtx_lock(&MutexT);
				PT->Elapsed = (float)(TCPst.Elapsed()) / 1000;		//Elapsed time
				TotalLen = TotalLen + recv_len;
				PT->Pkts = (unsigned long)TotalLen / PktSize;		//Packet number to be print out
				PT->TRate = (long double)TotalLen /(double) (PT->Elapsed*1024*1024);  //Transmission rate	
				//Compute Jitter time 
				if (n == 0){
					PT->JNew = 0;
					PT->JOld = PT->JNew;
					Told = 0;
				}
				else{
					Tn = (float)PT->Elapsed / n;
					Ti = (float)PT->Elapsed - Told;
					PT->JNew = (float)(PT->JOld * (n - 1) + fabs(Ti - Tn)) / n;  //Jitter to be printed out
					PT->JOld = PT->JNew;
					Told = PT->Elapsed;
				}
				n++;
				mtx_unlock(&MutexT);
			}
		}
	}
	thrd_join(TCP, NULL);
	free(ReceiveBuf);
	closesocket(s);
	WSACleanup();
	return 0;
}

int Display2(void* Ptr){
	struct Para* data = (struct Para *)Ptr;
	while (1){
		mtx_lock(&MutexT);
		printf("\rElapsed[%.1fs] Pkts[%d] Lost[%d, %.2f%%] Rate[%.2fMbps] Jitter[%.4fms]", data->Elapsed, data->Pkts, data->PktLost, data->LostPtg, data->TRate, (1000 * data->JNew));
		mtx_unlock(&MutexT);
		Sleep(data->RfTime);
	}
	return 0;
}