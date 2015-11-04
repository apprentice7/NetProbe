//TCPserver for use
//When the command line argument starts with r, this function will be called

#include "stdafx.h"


struct Thread{
	int RfTime;
	SOCKET TNewSock;
	//SOCKET UNewSock;
	struct sockaddr_in ClientAddr;
	//char *RecvBuf;
	int PacketSize;
	int ControlRate;
	int ControlNumber;
	int TCPNum;
	int UDPNum;
	int UPort;
	int Arr;
	double AggArray[10];
	double AggRate;
};
mtx_t MutexS;

int NetProbeServer(char *RefTime, char *TCPPort, char *UDPPort){

	//Intialize a new struct to store and pass data
	struct Thread DataT;
	struct Thread* Ptr;
	Ptr = &DataT;
	DataT.RfTime = atoi(RefTime);
	DataT.UPort = atoi(UDPPort);
	DataT.AggRate = 0;
	DataT.TCPNum = 0;
	DataT.UDPNum = 0;
	DataT.Arr = 0;
	DataT.PacketSize = 0;
	DataT.ControlRate = 0;
	DataT.ControlNumber = 0;
	for (int j = 0; j < 10; j++){
		DataT.AggArray[j] = 0;
	}

	/*Display thread*/

	thrd_t DispThread;
	if (thrd_create(&DispThread, ServerDisp, (void*)Ptr) == thrd_error){
		printf("Display thread is failed\n");
		return 0;
	}
	mtx_init(&MutexS, mtx_plain);

	/*UDP thread*/

	thrd_t UDPServer;
	if (thrd_create(&UDPServer, UDPS, (void*)Ptr) == thrd_error){
		printf("Create thread is failed\n");
		return 0;
	}

	/*TCP thread*/

	//Initialize socket parameters


	SOCKET Socket, new_socket;
	struct sockaddr_in server, client;
	int  recv_len;
	socklen_t c;
	c = sizeof(struct sockaddr_in);
	char *ReceiveBuf;
	ReceiveBuf = (char *)malloc(sizeof(char)* 1024);

	//Initialize Winsock
#ifdef WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed to Initialize, Error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
#endif
	if ((Socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
		printf("Could not create socket : d%", WSAGetLastError());
	}
	memset(&server, 0, sizeof(struct sockaddr_in));
	memset(&client, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(TCPPort));
	if (bind(Socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	listen(Socket, 10);

	//Listening to new TCP connection
	while (1){
		memset(ReceiveBuf, 0, 1024);
		if ((new_socket = accept(Socket, (struct sockaddr *)&client, &c)) == INVALID_SOCKET){
			printf("accept failed with error code: %d\n", WSAGetLastError());
			return 0;
		}
		else{
			DataT.TNewSock = new_socket;
			thrd_t TCPSThread;
			if (thrd_create(&TCPSThread, TCPChild, (void*)Ptr) == thrd_error){
				printf("Create thread is failed\n");
				return 0;
			}
			DataT.Arr = DataT.Arr + 1;
		}
	}
	closesocket(Socket);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}



int TCPChild(void* Ptr){
	struct Thread* PT = (struct Thread *)Ptr;

	//Initialize parameters
	int Recv = 0;
	int PktSize = 0;
	int ContrlRate = 0;
	int ContrlPkt = 0;
	int Send = 0;
	int ArrIndex = PT->Arr;
	long Pkts = 0;
	double Time = 0;
	double ThreadRate = 0;
	double StdElapsed = 0;
	double SlpTime = 0;
	SOCKET TCPSock;
	TCPSock = PT->TNewSock;

	mtx_lock(&MutexS);
	PT->TCPNum = PT->TCPNum + 1;	//TCP client number
	mtx_unlock(&MutexS);

	char *ReceiveBuf;
	char *SendBuf;
	ReceiveBuf = (char *)malloc(sizeof(char)* 1024);

	while (1){
		if ((Recv = recv(TCPSock, ReceiveBuf, 1024, 0)) == SOCKET_ERROR){
			printf("recv() failed with error code: %d\n", WSAGetLastError());
			return 0;
		}
		else{
			memcpy(&PktSize, ReceiveBuf, sizeof(int));
			memcpy(&ContrlRate, ReceiveBuf + 4, sizeof(int));
			memcpy(&ContrlPkt, ReceiveBuf + 8, sizeof(int));
			break;
		}
	}

	//Start a timer
	ES_FlashTimer TCPst = ES_FlashTimer();
	TCPst.Start();

	//Send data to client
	SendBuf = (char *)malloc(sizeof(char)* PktSize);
	while (1){
		/* Insert data to Send buffer here*/
		if ((Send = send(TCPSock, SendBuf, PktSize, 0)) == SOCKET_ERROR){
			printf("\nConnection failed with error code: %d\n", WSAGetLastError());
			break;
		}
		else{
			mtx_lock(&MutexS);
			Time = (double)TCPst.Elapsed() / 1000;	//Elapsed time in second
			Pkts = Pkts + 1;
			ThreadRate = (double)Pkts * PktSize / (double)(Time * 1024 * 1024);	//Transmission rate
			PT->AggArray[ArrIndex] = ThreadRate; //Transmitting rate
			mtx_unlock(&MutexS);
			//Control rate
			if (ContrlRate != 0){
				StdElapsed = (long double)Pkts * PktSize / ContrlRate;
				if (StdElapsed > Time){
					SlpTime = 1000 * (StdElapsed - Time);
					Sleep(SlpTime);
				}
			}
			//Control Packet Number
			if (ContrlPkt != 0){
				if (Pkts >= ContrlPkt){
					printf("Transmission completed\n");
					break;
				}
			}
		}
	}
	PT->TCPNum = PT->TCPNum - 1;
	PT->AggArray[ArrIndex] = 0;
	//thrd_join(TCPChild, NULL);
	free(ReceiveBuf);
	free(SendBuf);
}



int UDPS(void * UDPPort){
	struct Thread* UPtr = (struct Thread *)UDPPort;

	//Initialize parameters
	SOCKET Socket;
	int PacketSize = 0;
	int ControlRate = 0;
	int ControlNumber = 0;
	struct sockaddr_in server, si_other;
	int  recv_len;
	socklen_t slen;
	slen = sizeof(si_other);
	char *ReceiveBuf;
	ReceiveBuf = (char *)malloc(sizeof(char)* 1024);


	//Initialize winsock
#ifdef WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed to Initialize, Error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
#endif
	if ((Socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		printf("Could not create socket : d%\n", WSAGetLastError());
	}
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(UPtr->UPort);
	if (bind(Socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Bind failed with error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//Keep listening UDP client
	while (1){
		memset(ReceiveBuf, '\0', 1024);
		if ((recv_len = recvfrom(Socket, ReceiveBuf, 1024, 0, (struct sockaddr *)&si_other, &slen)) == SOCKET_ERROR){
			printf("recvfrom() failed with error code : %d\n", WSAGetLastError());
			return 0;
		}
		else{
			UPtr->ClientAddr = si_other;
			memcpy(&PacketSize, ReceiveBuf, sizeof(int));
			memcpy(&ControlRate, (ReceiveBuf + 4), sizeof(int));
			memcpy(&ControlNumber, (ReceiveBuf + 8), sizeof(int));
			UPtr->PacketSize = PacketSize;
			UPtr->ControlRate = ControlRate;
			UPtr->ControlNumber = ControlNumber;
			thrd_t UDPSThread;
			if (thrd_create(&UDPSThread, UDPChild, (void*)UPtr) == thrd_error){
				printf("Create thread failed\n");
				return 0;
			}
			UPtr->Arr = UPtr->Arr + 1;
		}
	}
	closesocket(Socket);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}



int UDPChild(void* Ptr){
	struct Thread *Pt = (struct Thread*)Ptr;

	//Initialize parameters
	SOCKET UDPSend;
	socklen_t slen;
	int ArrIndex = Pt->Arr;
	int PktSize = Pt->PacketSize;
	int ContrlRate = Pt->ControlRate;
	int ContrlPkt = Pt->ControlNumber;
	int Sqn = 0;	//Sequential Number
	long Pkts = 0;
	double Time = 0;
	double ThreadRate = 0;
	double StdElapsed = 0;
	double SlpTime = 0;

	mtx_lock(&MutexS);
	Pt->UDPNum = Pt->UDPNum + 1;	//TCP client number
	mtx_unlock(&MutexS);
	struct sockaddr_in UDPAddr;
	UDPAddr = Pt->ClientAddr;
	slen = sizeof(UDPAddr);
	char *SendBuf;
	SendBuf = (char *)malloc(sizeof(char)*PktSize);

	//Initialize winsock
#ifdef WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed to Initialize, Error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
#endif
	if ((UDPSend = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		printf("Could not create socket : d%\n", WSAGetLastError());
	}

	//Start a timer
	ES_FlashTimer UDPst = ES_FlashTimer();
	UDPst.Start();

	//Send data to UDP client with sequential number
	while (1){
		memset(SendBuf, 0, PktSize);
		Sqn = Sqn + 1;
		memcpy(SendBuf, &Sqn, sizeof(int));		//allocate sequential number
		if (sendto(UDPSend, SendBuf, PktSize + 4, 0, (struct sockaddr *)&UDPAddr, slen) == SOCKET_ERROR){
			printf("sendto() failed. Error code : %d\n", WSAGetLastError());
			break;
		}
		else{
			mtx_lock(&MutexS);
			Time = (double)UDPst.Elapsed() / 1000;		//Elapsed time in second
			Pkts = Pkts + 1;
			ThreadRate = (double)Pkts * PktSize / (double)(Time * 1024 * 1024);	//Transmission rate
			Pt->AggArray[ArrIndex] = ThreadRate; //Transmitting rate
			mtx_unlock(&MutexS);
			//Control rate
			if (ContrlRate != 0){
				StdElapsed = (long double)Pkts * PktSize / ContrlRate;
				if (StdElapsed > Time){
					SlpTime = 1000 * (StdElapsed - Time);
					Sleep(SlpTime);
				}
			}
			//Control Packet Number
			if (ContrlPkt != 0){
				if (Pkts >= ContrlPkt){
					printf("Transmission completed\n");
					break;
				}
			}
		}
	}
	Pt->AggArray[ArrIndex] = 0;
	Pt->UDPNum = Pt->UDPNum - 1;
	//thrd_join(UDPChild, NULL);
	free(SendBuf);
}



int ServerDisp(void* Ptr){
	struct Thread* data = (struct Thread *)Ptr;
	while (1){
		mtx_lock(&MutexS);
		for (int i = 0; i < 10; i++){
			data->AggRate = data->AggArray[i] + data->AggRate;
		}
		printf("\rAggregate[%.2fMbps] # of TCP Clients[%d] # of UDP Clients[%d]", data->AggRate, data->TCPNum, data->UDPNum);
		fflush(stdout);
		mtx_unlock(&MutexS);
		data->AggRate = 0;
		Sleep(data->RfTime);
	}
	return 0;
}
