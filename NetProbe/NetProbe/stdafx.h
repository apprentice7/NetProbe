// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef _NetProbe
#define _NetProbe


#pragma once
//#ifdef WIN32 // Windows
//#define public
#include "targetver.h"
#include <tchar.h>
#include <windows.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//#include "stdafx.h"
#include "tinycthread.h"
#include "es_TIMER.H"


// TODO: reference additional headers your program requires here

int NetProbeUDPS(char *RefTime, char *Hostname, char *Hostport, char *Packetsize, char* Rate, char* Num);

int NetProbeTCPS(char *RefTime, char *Hostname, char *Hostport, char *Packetsize, char* Rate, char* Num);

int NetProbeUDPR(char *RefTime, char *Hostname, char *Hostport, char *Packetsize);

int NetProbeTCPR(char *RefTime, char *Hostname, char *Hostport, char *Packetsize);

int NetProbeH(char* hostname);

int Display1(void* P);
int Display2(void* P);
int Display3(void* P);
int Display4(void* P);

// TODO: reference additional headers your program requires here
#endif //define(NetProbe) 


