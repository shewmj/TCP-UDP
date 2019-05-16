#include "Session.h"
#include "Application.h"
#include "Physical.h"



/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: Connect
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE : void Connect(string fileName)
--				string fileName: The path of the file to open for writing to
--
--RETURNS : void.
--
--NOTES :
--This function is called when a accept event is triggered from the listening socket. This function attempts to begin the connection with the socket s.
----------------------------------------------------------------------------------------------------------------------*/
void Connect(string fileName) {
	if (connected) {
		return;
	}
	connected = true;
	DWORD Ret;
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0) {
		MessageBox(hwnd, "WSAStartup failed", "Error", NULL);
		return;
	}
	if (strcmp(cs.TCP_UDP, "tcp") == 0) {
		if (strcmp(cs.CLIENT_SERVER, "server") == 0) {
			//TCP Server

			if ((Listen = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
				MessageBox(hwnd, "socket() failed", "Error", NULL);
				return;
			}
			memset((char *)&Server, 0, sizeof(struct sockaddr_in));
			Server.sin_family = AF_INET;
			Server.sin_addr.s_addr = htonl(INADDR_ANY);
			Server.sin_port = htons(cs.port);
			if (bind(Listen, (PSOCKADDR)&Server, sizeof(Server)) == SOCKET_ERROR) {
				MessageBox(hwnd, "bind() failed", "Error", NULL);
				return;
			}
			if (listen(Listen, 5)) {
				MessageBox(hwnd, "listen() failed", "Error", NULL);
				return;
			}
			oFile.open(fileName);
			if (!oFile.is_open()) {
				MessageBox(hwnd, "Open file failed", "Error", NULL);
				return;
			}
			WSAAsyncSelect(Listen, hwnd, WM_SOCKET, FD_ACCEPT);
			SetWindowText(hwnd, "Connected");

		}
		else {
			//TCP Client
			struct hostent	*hp;
			if ((Write = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				MessageBox(hwnd, "Cannot create socket", "Error", NULL);
				return;
			}
			memset((char *)&Server, 0, sizeof(struct sockaddr_in));
			Server.sin_family = AF_INET;
			Server.sin_port = htons(cs.port);
			if ((hp = gethostbyname(cs.ipAddress)) == NULL) {
				MessageBox(hwnd, "Can't get server's IP address", "Error", NULL);
				return;
			}
			memcpy((char *)&Server.sin_addr, hp->h_addr, hp->h_length);

			if (connect(Write, (struct sockaddr *)&Server, sizeof(Server)) == -1) {
				MessageBox(hwnd, "Can't connect to server", "Error", NULL); 
				return;
			}
			WSAAsyncSelect(Write, hwnd, WM_SOCKET, FD_WRITE);
		}
	} else {
		if (strcmp(cs.CLIENT_SERVER, "server") == 0) {
			//UDP Server
			if ((Listen = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
				MessageBox(hwnd, "Can't create socket", "Error", NULL);
				return;
			}
			memset((char *)&Server, 0, sizeof(Server));
			Server.sin_family = AF_INET;
			Server.sin_port = htons(cs.port);
			Server.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(Listen, (struct sockaddr *)&Server, sizeof(Server)) == -1) {
				MessageBox(hwnd, "bind failed", "Error", NULL);
				return;
			}
			oFile.open(fileName);
			if (!oFile.is_open()) {
				MessageBox(hwnd, "Could not open file", "Error", NULL);
				return;
			}
			WSAAsyncSelect(Listen, hwnd, WM_SOCKET, FD_READ);
			SetWindowText(hwnd, "Connected");

		}
		else {
			//UDP Client
			struct hostent	*hp;
			if ((Write = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
				MessageBox(hwnd, "Can't create a socket", "Error", NULL);
				return;
			}
			WSAAsyncSelect(Write, hwnd, WM_SOCKET, FD_WRITE);
			memset((char *)&Server, 0, sizeof(Server));
			Server.sin_family = AF_INET;
			Server.sin_port = htons(cs.port);
			if ((hp = gethostbyname(cs.ipAddress)) == NULL) {
				MessageBox(hwnd, "Can't get server's IP address", "Error", NULL);
				return;
			}
			memcpy((char *)&Server.sin_addr, hp->h_addr, hp->h_length);
			memset((char *)&Client, 0, sizeof(Client));
			Client.sin_family = AF_INET;
			Client.sin_port = htons(0);
			Client.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(Write, (struct sockaddr *)&Client, sizeof(Client)) == -1) {
				MessageBox(hwnd, "bind failed", "Error", NULL);
				return;
			}
		}
	}
}




/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: AcceptConnection
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE : void AcceptConnection(SOCKET s)
--				Socket s: The socket to accept a connection to
--
--RETURNS : void.
--
--NOTES :
--This function is called when a accept event is triggered from the listening socket. This function attempts to begin the connection with the socket s.
----------------------------------------------------------------------------------------------------------------------*/
void AcceptConnection(SOCKET s) {
	if (!started) {
		GetSystemTime(&stStartTime);
		started = true;
	}
	SOCKET Accept;
	if ((Accept = accept(s, NULL, NULL)) == INVALID_SOCKET) {
		MessageBox(hwnd, "accept() failed", "Error", NULL);
		return;
	}
	WSAAsyncSelect(Accept, hwnd, WM_SOCKET, FD_READ | FD_CLOSE);
}


