//Client function
//
//
//

#include "Header.h"

int Client(char *Hostname, char *port, char *Directory){
	int count = 0;
	//File directory setup
	DirCheck(Directory);
	char * CloudDir = (char*)malloc(sizeof(char)* 50);
	char * CloudDirMonitor = (char*)malloc(sizeof(char)* 50);
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
	//printf("clouddir is %s \n", CloudDir);
	//printf("clouddirMonitor is %s \n", CloudDirMonitor);
	USES_CONVERSION;
	TCHAR * Dir = A2T(CloudDir);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(Dir, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE){
		printf("No file in this directory\n");
	}
	if (!FindNextFile(hFind, &FindFileData)){
		printf("No next file in this directory\n");
	}
	if (!FindNextFile(hFind, &FindFileData)){
		printf("No next file in this directory\n");
	}

	char* FileDir = (char*)malloc(sizeof(char)* 1024);
	FileDir[0] = '\0';
	//Find all LWT for file except . and ..
	while (hFind != INVALID_HANDLE_VALUE){
		SYSTEMTIME stUTC, stLocal;
		DWORD dwRet;
		TCHAR lpszString[MAX_PATH];
		FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		TCHAR Datebuf[50];
		TCHAR Timebuf[50];
		GetDateFormat(LOCALE_USER_DEFAULT, 
			NULL, 
			&stLocal, 
			NULL, 
			Datebuf, 
			ARRAYSIZE(Datebuf));
		GetTimeFormat(LOCALE_USER_DEFAULT, 
			TIME_FORCE24HOURFORMAT, 
			&stLocal, 
			NULL, 
			Timebuf, 
			ARRAYSIZE(Timebuf));
		char *Date = (char*)malloc(sizeof(char)* 50);
		char *Time = (char*)malloc(sizeof(char)* 50);
		char* LWT = (char*)malloc(sizeof(char)* 12);
		Time = T2A(Timebuf);
		Date = T2A(Datebuf);

		LWT = GetLWT(Date, Time);
		char *Name = (char*)malloc(sizeof(char)* 50);
		Name = T2A(FindFileData.cFileName);
		//printf("Name is: %s\n", Name);
		//printf("LWT is %s\n", LWT);
		count++;
		strcat(FileDir, Name);
		strcat(FileDir, " ");
		strcat(FileDir, LWT);
		strcat(FileDir, "\n");
		//printf("%s\n", FileDir);
		//printf("%d\n", count);
		if (!FindNextFile(hFind, &FindFileData))
		{
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}

	//Number of files in local directory
	char Filecount[4];
	itoa(count, Filecount, 10);
	//printf("convert:%s\n", Filecount);

	//Protocal Header
	static char* AllFiles = "POST / HTTP/1.1\n";
	//all files
	//POST / HTTP/1.1
	//<FILE NUMBER>
	//<FILE NAME> <LAST WRITE TIME>
	//...

	static char* DownLoad = "POST RQ HTTP/1.1\n";

	static char* Upload = "POST UP HTTP/1.1\n";
	//UP
	//POST UP HTTP/1.1
	//<FILE NAME> <FILE SIZE>

	//static char* DownLoad = "POST RQ HTTP/1.1\n";


	//Initialize socket parameters
	SOCKET SendSocket, NewSocket;
	struct sockaddr_in server, client;
	int  RecvLen;
	//Statistics parameters
	int SendLen;
	char *SendBuf;
	char *ReceiveBuf;
	int PktSize = 1024;
	ReceiveBuf = (char *)malloc(sizeof(char)*PktSize);
	SendBuf = (char *)malloc(sizeof(char)*PktSize);

	//Fill the file buffer
	strcpy(SendBuf, AllFiles);
	strcat(SendBuf, Filecount);
	strcat(SendBuf, "\n");
	strcat(SendBuf, FileDir);
	//printf("SendBuf:%s\n", SendBuf);



	


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
	server.sin_addr.s_addr = inet_addr(Hostname);
	server.sin_port = htons(atoi(port));
	//Connect to remote server
	if (connect(SendSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Connect error\n");
		exit(EXIT_FAILURE);
	}

	//Send request to server
	if (SendLen = send(SendSocket, SendBuf, PktSize, 0) == SOCKET_ERROR){
		printf("Failed to request\n");
		exit(EXIT_FAILURE);
	}

	//NewSocket = accept(SendSocket, (struct sockaddr *)&client, &c);

	//Create New thread to Monitor local directory
	/*
	char *Ptr = CloudDirMonitor;
	thrd_t TCPSThread;
	if (thrd_create(&TCPSThread, MonLocalDir, (void*)Ptr) == thrd_error){
		printf("Create thread is failed\n");
		return 0;
	}
	*/

	int FileLength = 0;
	int SentLength = 0;
	int ReadLength = 0;
	char *Method;
	char *URI;
	char *Version;
	char FileLengthchar[10];
	char UploadFile[100];
	char ContentBuf[1025];
	Method = (char *)malloc(sizeof(char)* 20);
	URI = (char *)malloc(sizeof(char)* 256);
	Version = (char *)malloc(sizeof(char)* PktSize);
	chdir(CloudDirMonitor); // change directory to server\cloud
	FILE * FileHandle;

	while (1){
		memset(ReceiveBuf, 0, PktSize);
		if ((RecvLen = recv(SendSocket, ReceiveBuf, PktSize, 0)) == SOCKET_ERROR){
			printf("recv() failed with error code: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		else{
			printf("Buf: %s\n", ReceiveBuf);
			sscanf(ReceiveBuf, "%s %s %s", Method, URI, Version);
			if (strcmp(Method, "POST") == 0){
				if (strcmp(URI, "RQUP") == 0){ //require upload file
					ReceiveBuf = ReceiveBuf + 19;
					sscanf(ReceiveBuf, "%s", UploadFile);
					FileHandle = fopen(UploadFile, "rb");
					fseek(FileHandle, 0, SEEK_END);
					FileLength = ftell(FileHandle);
					itoa(FileLength, FileLengthchar, 10);
					fseek(FileHandle, 0, SEEK_SET);
					memset(SendBuf, 0, PktSize);
					strcpy(SendBuf, Upload);
					strcat(SendBuf, UploadFile);
					strcat(SendBuf, " ");
					strcat(SendBuf, FileLengthchar);
					strcat(SendBuf, " ");
					printf("buffer is here: %s\n", SendBuf);

					if ((SendLen = send(SendSocket, SendBuf, strlen(SendBuf), 0)) == SOCKET_ERROR){
						printf("\nConnection failed with error code: %d\n", WSAGetLastError());
						break;
					}
					else{
						/*debug info*/
						//printf("send0 is finished\n");
						//break;
					}
					//SendFile(SentLength,ReadLength,FileLength,ContentBuf,FileHandle,SendSocket);
					//printf("SL:%d\n", FileLength);
					
					while (1){
						if (SentLength < FileLength){
							memset(ContentBuf, 0, PktSize);
							fseek(FileHandle, SentLength, SEEK_SET);
							ReadLength = fread(ContentBuf, sizeof(char), 1024, FileHandle);
							printf("readbuf:%s\n", ContentBuf);
							printf("read0:%d\n", ReadLength);
							ReadLength = send(SendSocket, ContentBuf, ReadLength, 0);
							printf("sendbuf:%s\n", ContentBuf);
							printf("read:%d\n", ReadLength);
							printf("file:%d\n", FileLength);
							SentLength = SentLength + ReadLength;
						}
						else{
							break;
						}
					}
					
					//printf("done\n");
					fclose(FileHandle);
					SentLength = 0;
					closesocket(SendSocket);
					//printf("up:%s\n",UploadFile);
				}
				else if (strcmp(URI, "RQDOWN") == 0){
					printf("required download\n");
				}
			}
			else{

			}


		}
	}
	while (1){
		;
	}
	free(ReceiveBuf);
	closesocket(SendSocket);
#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}

int MonLocalDir(void* Ptr){
	USES_CONVERSION;
	char *CloudDir = (char *)Ptr;

	//Get handle for directory to be monitored
	HANDLE hDir = CreateFile(
		A2T(CloudDir),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
		);

	//Monitor local directory for amending files
	DWORD dwBytesReturned = 0;
	size_t nBufSize = 32 * 1024;
	FILE_NOTIFY_INFORMATION* pBuffer = (FILE_NOTIFY_INFORMATION*)calloc(1, nBufSize);
	FILE_NOTIFY_INFORMATION* pBufferCurrent;
	while (ReadDirectoryChangesW(hDir,
		pBuffer,
		nBufSize,
		FALSE,
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
		&dwBytesReturned,
		NULL,
		NULL
		)){
		pBufferCurrent = pBuffer;
		while (pBufferCurrent)
		{
			switch (pBufferCurrent->Action){
			case FILE_ACTION_ADDED:{
					printf("add\n");
					break;
			}
			case FILE_ACTION_REMOVED:{
					printf("remove\n");
					break;
			}
			case FILE_ACTION_MODIFIED:{
					printf("modify\n");
					break;
			}
			case FILE_ACTION_RENAMED_OLD_NAME:{
					printf("old\n");
					break;
			}
			case FILE_ACTION_RENAMED_NEW_NAME:{
					printf("new\n");
					break;
			}
			default:{
					printf("failed\n");
					break;
			}
			}
			char *FileName = (char *)malloc(sizeof(char)* 1024);
			wcstombs(FileName, pBufferCurrent->FileName, 1024);
			FileName[pBufferCurrent->FileNameLength / 2] = '\0';
			printf("file:%s\n", FileName);
			if (pBufferCurrent->NextEntryOffset){
				pBufferCurrent = (FILE_NOTIFY_INFORMATION*)(((BYTE*)pBufferCurrent) + pBufferCurrent->NextEntryOffset);
			}
			else{
				pBufferCurrent = NULL;
			}
		}
	}
	return 0;
}





//Get last write time and assign to an interger
char* GetLWT(char *Date, char *Time){

	char Day[3];
	char Month[3];
	char Hour[3];
	char Minute[3];
	char Second[3];
	//int lastwritetime = 0;
	char *LastWriteTime = (char*)malloc(sizeof(char)* 50);
	if (Date[1] == '/'){
		Day[0] = '0';
		Day[1] = Date[0];
		Day[2] = '\0';
		if (Date[3] == '/'){
			Month[0] = '0';
			Month[1] = Date[2];
			Month[2] = '\0';
		}
		else{
			Month[0] = Date[2];
			Month[1] = Date[3];
			Month[2] = '\0';
		}
	}
	else{
		Day[0] = Date[0];
		Day[1] = Date[1];
		Day[2] = '\0';

		if (Date[4] == '/'){
			Month[0] = '0';
			Month[1] = Date[3];
			Month[2] = '\0';
		}
		else{
			Month[0] = Date[3];
			Month[1] = Date[4];
			Month[2] = '\0';
		}
	}
	if (Time[1] == ':'){
		Hour[0] = '0';
		Hour[1] = Time[0];
		Hour[2] = '\0';
		Minute[0] = Time[2];
		Minute[1] = Time[3];
		Minute[2] = '\0';
		Second[0] = Time[5];
		Second[1] = Time[6];
		Second[2] = '\0';
	}
	else{
		Hour[0] = Time[0];
		Hour[1] = Time[1];
		Hour[2] = '\0';
		Minute[0] = Time[3];
		Minute[1] = Time[4];
		Minute[2] = '\0';
		Second[0] = Time[6];
		Second[1] = Time[7];
		Second[2] = '\0';
	}

	strcpy(LastWriteTime, Month);
	strcat(LastWriteTime, Day);
	strcat(LastWriteTime, Hour);
	strcat(LastWriteTime, Minute);
	strcat(LastWriteTime, Second);
	//printf("%s\n", LastWriteTime);
	//lastwritetime = atoi(LastWriteTime);
	return LastWriteTime;
}



