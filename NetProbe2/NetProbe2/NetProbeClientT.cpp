//TCPserver for use
//When the command line argument starts with r, this function will be called

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

mtx_t MutexT;

int NetProbeClientT(char *RefTime, char *ServerHost, char *ServerPort, char* PacketSize, char* Rate, char* Num){

	//Initialize socket parameters
	SOCKET SendSocket, NewSocket;
	struct sockaddr_in server, client;
	int c, recv_len;
	c = sizeof(struct sockaddr_in);

	//Statistics parameters
	int SendLen;
	char *SendBuf;
	char *ReceiveBuf;
	int RfTime = atoi(RefTime);
	int PktSize = atoi(PacketSize);
	int ContrlRate = atoi(Rate);
	int ContrlPkt = atoi(Num);
	unsigned long long TotalLen = 0;
	ReceiveBuf = (char *)malloc(sizeof(char)*PktSize);
	SendBuf = (char *)malloc(sizeof(char)*PktSize);

	//Intialize a new struct to store data
	struct Para DataT;
	struct Para* PT;
	PT = &DataT;
	DataT.RfTime = RfTime;
	DataT.PktLimit = ContrlPkt;
	DataT.Elapsed = 0;
	DataT.Pkts = 0;
	DataT.PktLost = 0;
	DataT.LostPtg = 0;
	DataT.TRate = 0;

	//Intialize a new thread
	thrd_t TCP;
	if (thrd_create(&TCP, TClientDisp, (void*)PT) == thrd_error){
		printf("Display thread is failed\n");
		exit(EXIT_FAILURE);
	}
	mtx_init(&MutexT, mtx_plain);

	//Initialize Winsock
#ifdef WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed to Initialize, Error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
#endif
	if ((SendSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
		printf("Could not create socket : d%", WSAGetLastError());
	}
	memset(&server, 0, sizeof(struct sockaddr_in));
	memset(&client, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ServerHost);
	server.sin_port = htons(atoi(ServerPort));
	//Connect to remote server
	if (connect(SendSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Connect error\n");
		exit(EXIT_FAILURE);
	}

	//Send parameters to server through buffer
	memcpy(SendBuf, &PktSize, sizeof(int));
	memcpy((SendBuf + 4), &ContrlRate, sizeof(int));
	memcpy((SendBuf + 8), &ContrlPkt, sizeof(int));
	if (SendLen = send(SendSocket, SendBuf, PktSize, 0) == SOCKET_ERROR){
		printf("Failed to request\n");
		exit(EXIT_FAILURE);
	}

	//NewSocket = accept(SendSocket, (struct sockaddr *)&client, &c);

	//start the timer
	ES_FlashTimer TCPst = ES_FlashTimer();
	TCPst.Start();

	while (1){
		memset(ReceiveBuf, 0, PktSize);
		if ((recv_len = recv(SendSocket, ReceiveBuf, PktSize, 0)) == SOCKET_ERROR){
			printf("recv() failed with error code: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		else{
			mtx_lock(&MutexT);
			PT->Elapsed = (float)(TCPst.Elapsed()) / 1000;		//Elapsed time
			TotalLen = TotalLen + recv_len;
			PT->Pkts = (unsigned long)TotalLen / PktSize;		//Packet number to be print out
			PT->TRate = (long double)TotalLen / (double)(PT->Elapsed * 1024 * 1024);  //Transmission rate	
			mtx_unlock(&MutexT);
		}
	}

	thrd_join(TCP, NULL);
	free(ReceiveBuf);
	closesocket(SendSocket);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}

int TClientDisp(void* Ptr){
	struct Para* data = (struct Para *)Ptr;
	while (1){
		mtx_lock(&MutexT);
		printf("\rElapsed[%.1fs] Pkts[%d] Lost[%d, %.2f%%] Rate[%.2fMbps]", data->Elapsed, data->Pkts, data->PktLost, data->LostPtg, data->TRate);
		fflush(stdout);
		mtx_unlock(&MutexT);
		Sleep(data->RfTime);
	}
	return 0;
}

