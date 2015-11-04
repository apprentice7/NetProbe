//TCPserver for use
//When the command line argument starts with r, this function will be called

#include "stdafx.h"

//Queue ADT to supply the thread pool mode
typedef struct Node{
	SOCKET item;
	struct Node* next;
}Node;
typedef struct Queue{
	Node* head;
	Node* tail;
	void(*Enqueue)(struct Queue*, SOCKET);
	SOCKET(*Dequeue)(struct Queue*);
	SOCKET(*Front)(struct Queue*);
	int size;	
}Queue;

void Enqueue(Queue* queue, SOCKET item);
SOCKET Dequeue(Queue* queue);
SOCKET Front(Queue* queue);
 
Queue createQueue(){
	Queue queue;
	queue.size = 0;
	queue.head = NULL;
	queue.tail = NULL;
	queue.Enqueue = &Enqueue;
	queue.Dequeue = &Dequeue;
	queue.Front = &Front;
	return queue;
}

void Enqueue(Queue* queue, SOCKET item){
	Node* n = (Node*)malloc(sizeof(Node));
	n->item = item;
	n->next = NULL;
	if (queue->head == NULL){
		queue->head = n;
	}
	else{
		queue->tail->next = n;
	}
	queue->tail = n;
	queue->size++;
}
SOCKET Dequeue(Queue* queue){
	Node* head = queue->head;
	SOCKET item = head->item;
	queue->head = head->next;
	queue->size--;
	free(head);
	return item;
}
SOCKET Front(Queue* queue){
	Node* head = queue->head;
	return head->item;
}

//Initialize a queue 
Queue SocketQueue = createQueue();

mtx_t ThMutex;
cnd_t cond;

int NetProbeServerTP(char *TCPPort, char *threadnum){

	//Create a thread pool
	cnd_init(&cond);
	mtx_init(&ThMutex, mtx_plain);
	int ThreadNum = atoi(threadnum);
	
	for (int j = 0; j < ThreadNum; j++){
		thrd_t TCPSThread;
		if (thrd_create(&TCPSThread, TPTCPChild, NULL) == thrd_error){
			printf("Create thread is failed\n");
			return 0;
		}
		else{
			printf("Creating thread %d in threadpool mode.\n", j+1);
			if (j == ThreadNum-1){
				printf("Create threadpool with %d threads in totoal.\n", j+1);
				printf("Waiting for client to connect...\n");
			}
		}
	}
	
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
	listen(Socket, 100);
	
	//Listening to new TCP connection
	while (1){
		memset(ReceiveBuf, 0, 1024);
		if ((new_socket = accept(Socket, (struct sockaddr *)&client, &c)) == INVALID_SOCKET){
			printf("accept failed with error code: %d\n", WSAGetLastError());
			return 0;
		}
		else{
			mtx_lock(&ThMutex);
			SocketQueue.Enqueue(&SocketQueue, new_socket);
			mtx_unlock(&ThMutex);
			cnd_signal(&cond);
		}
		Sleep(2);
	}
	closesocket(Socket);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}



int TPTCPChild(void* Ptr){

	static char* SuccessResponse =
		" 200 OK\n"
		"Content-type: text/html\n"
		"Connection: close\n"
		"\n";
	static char *NotFoundResponse =
		" 404 Not Found\n"
		"Content-type: text/html\n"
		"Connection: close\n"
		"\n";
	static char* BadMethodResponse =
		" 501 Method Not Implemented\n"
		"Content-type: text/html\n"
		"Connection: close\n"
		"\n";
	
	int Recv = 0;
	int Send = 0;
	int SentLength = 0;
	int ReadLength = 0;
	int FileLength = 0;
	int PktSize = 1024;

	char *Method;
	char *URI;
	char *Version;
	char *ReceiveBuf;
	char *HeaderBuf;
	char ContentBuf[1025];
	char * FileBuffer = 0;
	
	Method = (char *)malloc(sizeof(char)* 20);
	URI = (char *)malloc(sizeof(char)* 256);
	Version = (char *)malloc(sizeof(char)* PktSize);
	HeaderBuf = (char *)malloc(sizeof(char)* PktSize);
	ReceiveBuf = (char *)malloc(sizeof(char)* PktSize);
	
	FILE * f;
	SOCKET TCPSock;
	
	while (1){
		mtx_lock(&ThMutex);
		if (SocketQueue.size == 0){
			cnd_wait(&cond, &ThMutex);
		}
		TCPSock = SocketQueue.Dequeue(&SocketQueue);
		mtx_unlock(&ThMutex);

		while (1){
			if ((Recv = recv(TCPSock, ReceiveBuf, PktSize, 0)) == SOCKET_ERROR){
				printf("recv() failed with error code: %d\n", WSAGetLastError());
				return 0;
			}
			else{
				sscanf(ReceiveBuf, "%s %s %s", Method, URI, Version);
				if (strcmp(Method, "GET") == 0){
					
					if (strcmp(URI, "/") == 0 || strcmp(URI, "/index.html") == 0){
						f = fopen("index.html", "r");
						strcat(Version, SuccessResponse);
					}
					else if (strcmp(URI, "/readme.html") == 0){
						f = fopen("readme.html", "r");
						strcat(Version, SuccessResponse);
					}
					else{
						f = fopen("404error.html", "r");
						strcat(Version, NotFoundResponse);
					}
				}
				else{
					f = fopen("404error.html", "r");
					strcat(Version, BadMethodResponse);
				}
				fseek(f, 0, SEEK_END);
				FileLength = ftell(f);
				fseek(f, 0, SEEK_SET);
				
				HeaderBuf = Version; //Store the header file
				while (1){
					if ((Send = send(TCPSock, HeaderBuf, strlen(HeaderBuf), 0)) == SOCKET_ERROR){
						printf("\nConnection failed with error code: %d\n", WSAGetLastError());
						break;
					}
					else{
						/*debug info*/
						//printf("send0 is finished\n");
						break;
					}
				}
				while (SentLength < FileLength){
					ReadLength = fread(ContentBuf, sizeof(char), 1024, f);
					ReadLength = send(TCPSock, ContentBuf, ReadLength, 0);
					SentLength = SentLength + ReadLength;
				}
				fclose(f);
				SentLength = 0;
				//Sleep(500);
				closesocket(TCPSock);
				break;
			}
		}
	}
}
