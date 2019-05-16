#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include <Windows.h>
#include "resource.h"
#include "winmenu.h"
#include <fstream> 


#define DATA_BUFSIZE 60000

using namespace std;

struct CommSettings {
	char *TCP_UDP;
	char *CLIENT_SERVER;
	char ipAddress[30];
	int dataSize;
	int port;
	int sendCount;
};


extern HWND hwnd;

extern WSADATA wsaData;
extern CommSettings cs;

extern ofstream oFile;
extern ifstream iFile;

extern SYSTEMTIME stStartTime, stEndTime;
extern bool started;

extern SOCKET Listen;
extern SOCKET Write;
extern SOCKADDR_IN Server;
extern SOCKADDR_IN Client;

extern bool connected;
extern int packets;
extern int dataReceived;

extern char buf[DATA_BUFSIZE];
