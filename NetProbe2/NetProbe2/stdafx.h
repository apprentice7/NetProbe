// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef _NetProbe
#define _NetProbe

#pragma once

#ifdef WIN32  // Windows
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "targetver.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tinycthread.h"
#include "es_TIMER.H"
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
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

// TODO: reference additional headers your program requires here

int NetProbeServer(char *RefTime, char *TCPPort, char *UDPPort);

int NetProbeClientT(char *RefTime, char *ServerHost, char *ServerPort, char* PacketSize, char* Rate, char* Num);

int NetProbeClientU(char *RefTime, char *ServerHost, char *ServerPort, char* PacketSize, char* Rate, char* Num);

int UClientDisp(void* P);

int TClientDisp(void* P);

int ServerDisp(void* P);

int UDPChild(void* P);

int UDPS(void* P);

int TCPChild(void* P);


// TODO: reference additional headers your program requires here
#endif //define(NetProbe) 


