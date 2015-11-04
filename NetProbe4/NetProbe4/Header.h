// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef _NetProbe
#define _NetProbe

#pragma once



#ifdef WIN32  // Windows
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tinycthread.h"
#include "es_TIMER.H"
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "atlstr.h"
#include <strsafe.h>

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include "atlstr.h"
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tinycthread.h"
#include "es_TIMER.H"
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SoCKEADDR sockaddr
#define WSAGetLastError() (errno)
#define closesocket(s) close(s)
#define ioctlsocket ioctl
#define WSAEWOULDBLOCK EWOULDBLOCK
#define Sleep(s) usleep(s*1000)

#endif

int Server(char *Port, char *Directory);

int Client(char *Hostname, char *Port, char *Directory);

int TCPChild(void* P);

void DirCheck(char * P);

char* GetLWT(char *Date, char *Time);

int MonLocalDir(void* Ptr);

int ReceiveFile(int RecvLen, int Recv, char *FileWSize, char *ReceivePacket, FILE *FileHandle,
	SOCKET TCPSock);

int SendFile(int SentLength, int ReadLength, int FileLength, char *ContentBuf, FILE *FileHandle,
	SOCKET SendSocket);

#endif //define(NetProbe) 


