//Get the host information


#include "stdafx.h"

int NetProbeH(char *hostname){
	// Declare and initialize variables
	WSADATA wsaData;
	int iResult;
	DWORD dwError;
	int i = 0;

	struct hostent *remoteHost;
	char *host_name;
	struct in_addr addr;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	host_name = hostname;
	remoteHost = gethostbyname(host_name);

	if (remoteHost == NULL) {
		dwError = WSAGetLastError();
		if (dwError != 0) {
			if (dwError == WSAHOST_NOT_FOUND) {
				printf("Host not found\n");
				return 1;
			}
			else if (dwError == WSANO_DATA) {
				printf("No data record found\n");
				return 1;
			}
			else {
				printf("Function failed with error: %ld\n", dwError);
				return 1;
			}
		}
	}
	else {
		printf("Official name: %s\n", remoteHost->h_name);

		i = 0;
		if (remoteHost->h_addrtype == AF_INET)
		{
			printf("IP address (es): {");
			while (remoteHost->h_addr_list[i] != 0) {
				addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
				printf("%s, ", inet_ntoa(addr));
			}
			printf("}");
		}
		else if (remoteHost->h_addrtype == AF_NETBIOS)
		{
			printf("NETBIOS address was returned\n");
		}
	}
	return 0;

}