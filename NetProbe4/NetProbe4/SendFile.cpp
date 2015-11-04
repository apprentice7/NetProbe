
#include "Header.h"

int SendFile(int SentLength, int ReadLength, int FileLength, char *ContentBuf, FILE *FileHandle,
	SOCKET SendSocket){
	while (1){
		if (SentLength < FileLength){
			memset(ContentBuf, 0, 1024);
			fseek(FileHandle, SentLength, SEEK_SET);
			ReadLength = fread(ContentBuf, sizeof(char), 1024, FileHandle);
			ReadLength = send(SendSocket, ContentBuf, ReadLength, 0);
			//printf("sendbuf:%s\n", ContentBuf);
			//printf("file:%d\n", FileLength);
			SentLength = SentLength + ReadLength;
		}
		else{
			break;
		}
	}
	return 0;
}