

#include "Header.h"

int ReceiveFile(int RecvLen, int Recv, char *FileWSize, char *ReceivePacket, FILE *FileHandle,
	SOCKET TCPSock){
	while (RecvLen < atoi(FileWSize)){
		memset(ReceivePacket, 0, 1024);
		fseek(FileHandle, RecvLen, SEEK_SET);
		Recv = recv(TCPSock, ReceivePacket, 1024, 0);
		//printf("receivebuf:%s\n", ReceivePacket);
		//printf("receive: %s", ReceivePacket);
		//if ()
		fwrite(ReceivePacket, sizeof(char), 1024, FileHandle);
		RecvLen = RecvLen + Recv;
		//printf("done\n");
	}
	return 0;
}