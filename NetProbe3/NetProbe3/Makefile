#the compiler to use
CC = g++
#CFLAGS
CFLAGS = -c
#LFLAGS
LFLAGS = -lrt -pthread 

all: NetProbe

NetProbe: NetProbe3.o NetProbeServerOD.o NetProbeServerTP.o tinycthread.o stdafx.o
	$(CC) $(LFLAGS) NetProbe3.o NetProbeServerOD.o NetProbeServerTP.o tinycthread.o stdafx.o -o NetProbe
NetProbe2.o: NetProbe3.cpp
	$(CC) $(CFLAGS) NetProbe3.cpp
NetProbeServer.o: NetProbeServerOD.cpp
	$(CC) $(CFLAGS) NetProbeServerOD.cpp
NetProbeClientU.o: NetProbeServerTP.cpp
	$(CC) $(CFLAGS) NetProbeServerTP.cpp
#NetProbeClientT.o: NetProbeClientT.cpp
#	$(CC) $(CFLAGS) NetProbeClientT.cpp
tinycthread.o: tinycthread.cpp
	$(CC) $(CFLAGS) tinycthread.cpp
stdafx.o: stdafx.cpp
	$(CC) $(CFLAGS) stdafx.cpp

clean:
	rm *o NetProbe

