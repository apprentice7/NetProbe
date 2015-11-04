//Server function
//
//
//

#include "Header.h"
//char *cwd;
char * CloudDirMonitor;
int Server(char *Port, char *Directory){

	DirCheck(Directory);
	CloudDirMonitor = (char*)malloc(sizeof(char)* 50);
	char * CloudDir = (char*)malloc(sizeof(char)* 50);
	strcpy(CloudDir, Directory);
	strcpy(CloudDirMonitor, Directory);
	int Len = strlen(Directory);
	if (CloudDir[Len - 1] == '\\'){
		CloudDir[Len - 1] = '\0';
	}
	if (CloudDirMonitor[Len - 1] == '\\'){
		CloudDirMonitor[Len - 1] = '\0';
	}
	strcpy(CloudDir + strlen(CloudDir), "\\cloud\\*.*"); //Directory for checking LWT
	strcpy(CloudDirMonitor + strlen(CloudDirMonitor), "\\cloud\\"); //Directory for monitoring
	//cwd = (char*)malloc(sizeof(char)* 261);
	//char buff[MAX_PATH + 1];
	//cwd = getcwd(buff, MAX_PATH + 1);
	//printf("dir: %s\n", cwd);
	//strcat(cwd, "\\");
	//strcat(cwd, CloudDirMonitor);
	//printf("dir: %s\n", cwd);
	//printf("clouddir is %s \n", CloudDir);
	//printf("clouddirMonitor is %s \n", CloudDirMonitor);
	//chdir(cwd); // change directory to server\cloud



	


















	//Socket and buffer initialization
	SOCKET *Ptr;
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
	server.sin_port = htons(atoi(Port));
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
			Ptr = &new_socket;
			thrd_t TCPSThread;
			if (thrd_create(&TCPSThread, TCPChild, (void*)Ptr) == thrd_error){
				printf("Create thread is failed\n");
				return 0;
			}
		}
		Sleep(2);
	}
	closesocket(Socket);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}



int TCPChild(void* Ptr){

	static char* RQUpload =
		"POST RQUP HTTP/1.1\n";

	static char *RQDownload =
		"POST RQDOWN HTTP/1.1\n";
		
	static char* BadMethodResponse =
		" 501 Method Not Implemented\n"
		"Content-type: text/html\n"
		"Connection: close\n"
		"\n";

	int Recv = 0;
	int SendLen = 0;
	int RecvLen = 0;
	//int ReadLength = 0;
	//int FileLength = 0;
	int PktSize = 1024;
	char FileNumber[8];
	char FileName[100];
	char LWT[12];

	char *Method;
	char *URI;
	char *Version;
	char *ReceiveBuf;
	char *ReceivePacket;
	char *ResponseBuf;
	char ContentBuf[1025];
	char * FileBuffer = 0;
	char *Fsize;
	char FileToWrite[100];
	char FileWSize[100];
	

	Method = (char *)malloc(sizeof(char)* 20);
	URI = (char *)malloc(sizeof(char)* 256);
	Version = (char *)malloc(sizeof(char)* PktSize);
	ResponseBuf = (char *)malloc(sizeof(char)* PktSize);
	ReceiveBuf = (char *)malloc(sizeof(char)* PktSize);
	ReceivePacket = (char *)malloc(sizeof(char)* PktSize);
	Fsize = (char *)malloc(sizeof(char)* 100);
	chdir(CloudDirMonitor); // change directory to server\cloud

	SOCKET TCPSock;
	TCPSock = *(SOCKET*)Ptr;
	FILE * FileHandle;
	struct tm *timer;
	int LocalLWT;
	

	while (1){
		if ((Recv = recv(TCPSock, ReceiveBuf, PktSize, 0)) == SOCKET_ERROR){
			printf("recv() failed with error code: %d\n", WSAGetLastError());
			return 0;
		}
		else{
			//printf("Buffer is:%s\n", ReceiveBuf);
			sscanf(ReceiveBuf, "%s %s %s %s", Method, URI, Version, FileNumber);
			if (strcmp(Method, "POST") == 0){
				if (strcmp(URI, "/") == 0){ //client send whole file directory
					if ((atoi(FileNumber)) < 10){
						ReceiveBuf = ReceiveBuf + 18;
					}
					else if ((atoi(FileNumber)) < 100){
						ReceiveBuf = ReceiveBuf + 19;
					}
					else{ //!!!File Number shall not be more than 9999
						ReceiveBuf = ReceiveBuf + 20;
					}
					sscanf(ReceiveBuf, "%s %s", FileName, LWT);
					if (strcmp(FileName, "..") == 0){
						printf("no file exist\n");
					}
					else{
						for (int i = 0; i < atoi(FileNumber) - 1; i++){
							ReceiveBuf = ReceiveBuf + strlen(FileName) + strlen(LWT) + 2;
							sscanf(ReceiveBuf, "%s %s", FileName, LWT);
							struct stat info;
							if (stat(FileName, &info) == 0){
								if (info.st_mode & S_IFREG){ //file exist
									printf("%s is exist\n",FileName);
									timer = localtime(&info.st_mtime);
									LocalLWT = timer->tm_sec +
										timer->tm_min * 100 +
										timer->tm_hour * 10000 +
										timer->tm_mday * 1000000 +
										(timer->tm_mon + 1) * 100000000;
									printf("Local:%d\n", LocalLWT);
									printf("remot:%d\n", atoi(LWT));
									if (atoi(LWT) > LocalLWT){ //client version is newer: Upload
										strcpy(ResponseBuf, RQUpload);
										strcat(ResponseBuf, FileName);
										//printf("send: %s\n", ResponseBuf);
										if ((SendLen = send(TCPSock, ResponseBuf, strlen(ResponseBuf), 0)) == SOCKET_ERROR){
											printf("\nConnection failed with error code: %d\n", WSAGetLastError());
											break;
										}
										else{
											memset(ReceivePacket, 0, PktSize);
											while (1){
												if ((Recv = recv(TCPSock, ReceivePacket, PktSize, 0)) == SOCKET_ERROR){
													printf("recv() failed with error code: %d\n", WSAGetLastError());
													return 0;
												}
												else{
													//printf("ddddd\n");
													sscanf(ReceivePacket, "%s %s %s", Method, URI, Version);
													if (strcmp(URI, "UP") == 0){
														ReceivePacket = ReceivePacket + 17;
														sscanf(ReceivePacket, "%s %s", FileToWrite, FileWSize);
														//printf("here! %s %d\n", FileToWrite, atoi(FileWSize));
														//break;

														FileHandle = fopen(FileToWrite, "w");
														
														//ReceiveFile(RecvLen, Recv, FileWSize, ReceivePacket, FileHandle, TCPSock);
														
														while (RecvLen < atoi(FileWSize)){
															memset(ReceivePacket, 0, PktSize);
															fseek(FileHandle, RecvLen, SEEK_SET);
															Recv = recv(TCPSock, ReceivePacket, 1024, 0);
															printf("receivebuf:%s\n", ReceivePacket);
															//printf("receive: %s", ReceivePacket);
															//if ()
															fwrite(ReceivePacket, sizeof(char), 1024, FileHandle);
															RecvLen = RecvLen + Recv;
															//printf("done\n");
														}
														
														//return 0;
														//printf("ddddd\n");
													}
													break;
												}
											}
										}
									}
									else{ //server version is newer: Download
										strcpy(ResponseBuf, RQDownload);
										strcat(ResponseBuf, FileName);
										itoa((int)info.st_size, Fsize, 10);
										strcat(ResponseBuf, " ");
										strcat(ResponseBuf, Fsize);
										//printf("sendbuf: %s\n", ResponseBuf);
									}
								}
							}
							else{ // file not exist in server
								printf("file does not exist\n");
							}
						}
					}
				}
				else if (strcmp(URI, "UP") == 0){
					ReceiveBuf = ReceiveBuf + 17;
					sscanf(ReceiveBuf, "%s %s", FileToWrite, FileWSize);
					//printf("%s %s", FileToWrite, FileWSize);
					//f = fopen(FileToWrite, "w");

				}
			}
			/*
			while (1){
				if ((Send = send(TCPSock, ResponseBuf, strlen(ResponseBuf), 0)) == SOCKET_ERROR){
					printf("\nConnection failed with error code: %d\n", WSAGetLastError());
					break;
				}
				else{
					//debug info
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
			*/
			//Sleep(500);
			closesocket(TCPSock);
			break;
		}
	}
}



