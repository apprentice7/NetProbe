#the compiler to use
CC = g++
#CFLAGS
CFLAGS = -c
#LFLAGS
LFLAGS = -lrt -pthread 

all: NetProbe

NetProbe: NetProbe2.o NetProbeServer.o NetProbeClientU.o NetProbeClientT.o tinycthread.o stdafx.o
	$(CC) $(LFLAGS) NetProbe2.o NetProbeServer.o NetProbeClientU.o NetProbeClientT.o tinycthread.o stdafx.o -o NetProbe
NetProbe2.o: NetProbe2.cpp
	$(CC) $(CFLAGS) NetProbe2.cpp
NetProbeServer.o: NetProbeServer.cpp
	$(CC) $(CFLAGS) NetProbeServer.cpp
NetProbeClientU.o: NetProbeClientU.cpp
	$(CC) $(CFLAGS) NetProbeClientU.cpp
NetProbeClientT.o: NetProbeClientT.cpp
	$(CC) $(CFLAGS) NetProbeClientT.cpp
tinycthread.o: tinycthread.cpp
	$(CC) $(CFLAGS) tinycthread.cpp
stdafx.o: stdafx.cpp
	$(CC) $(CFLAGS) stdafx.cpp

clean:
	rm *o NetProbe

