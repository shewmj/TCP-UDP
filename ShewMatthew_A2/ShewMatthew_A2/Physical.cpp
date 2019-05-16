#include "Session.h"
#include "Application.h"
#include "Physical.h"

int client_len = sizeof(Client);
/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: ReadFromSocket
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE : void ReadFromSocket(SOCKET s)
--				Socket s: The socket to read data from
--
--RETURNS : void.
--
--NOTES :
--This function is called when a read event is triggered from the bound socket s. This function reads in the data fromthe socket and writes the data to a file.
----------------------------------------------------------------------------------------------------------------------*/
void ReadFromSocket(SOCKET s) {
	if (strcmp(cs.TCP_UDP, "tcp") == 0) {
		//TCP
		int bytesToRead = DATA_BUFSIZE;
		int iter = 0;
		int n = 0;
		bool more = true;
		while (more) {
			while (bytesToRead > cs.dataSize) {
				n = recv(s, buf + iter, cs.dataSize, 0);
				dataReceived += n;
				packets++;
				if (n <= 0) {
					more = false;
					break;
				} else {
					bytesToRead -= n;
					iter += n;
				}
			}
			oFile.write(buf, iter);
			iter = 0;
			bytesToRead = DATA_BUFSIZE;
		}
		
		
	} else {
		//UDP
		int bytesToRead = DATA_BUFSIZE;
		int iter = 0;
		int n = 0;
		bool more = true;
		while (more) {
			while (bytesToRead > cs.dataSize) {
				if ((n = recvfrom(Listen, buf + iter, cs.dataSize + 1, 0, (struct sockaddr *)&Client, &client_len)) < 0) {
					perror("recvfrom error");
					exit(1);
				}
				dataReceived += n;
				packets++;
				if (n == 7) {
					if (strncmp(buf + iter - n, "t:9}m[", 7) == 0) {
						oFile.write(buf, iter - n);
						GetSystemTime(&stEndTime);
						char buffer[50];
						sprintf_s(buffer,
							"\nEnd Time %02d:%02d.%03d",
							stEndTime.wMinute,
							stEndTime.wSecond,
							stEndTime.wMilliseconds);
						oFile << buffer;
						oFile << "\nPackets: " << packets << "\nData Receveived: " << dataReceived << "\nAverage Packet Size: " << dataReceived / packets;
						packets = 0;
						dataReceived = 0;
						oFile.close();
						connected = false;
						closesocket(Listen);
						SetWindowText(hwnd, "Disconnected");
						MessageBox(hwnd, "here", "", 0);

					}
				}
				if (n <= 0) {
					more = false;
					break;
				}
				else {
					bytesToRead -= n;
					iter += n;
				}
			}
			oFile.write(buf, iter);
			iter = 0;
			bytesToRead = DATA_BUFSIZE;
		}
	}
}



/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: WriteToSocket
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE : void WriteToSocket(string fileName)
--				string fileName: the path of the file to be read fromet
--
--RETURNS : void.
--
--NOTES :
--This function is called when a write event is triggered from the connected socket. This function writes data from a file to the socket.
----------------------------------------------------------------------------------------------------------------------*/
void WriteToSocket(string fileName) {
	GetSystemTime(&stStartTime);
	ifstream myfile(fileName);
	char * buf = (char *)malloc(sizeof(char) * (cs.dataSize));
	memset(buf, '\0', cs.dataSize);
	int dataSent = 0;

	if (strcmp(cs.TCP_UDP, "tcp") == 0) {
		if (myfile.is_open()) {
			int size;
			while (!myfile.eof()) {
				myfile.read(buf, cs.dataSize);
				size = strlen(buf);

				for (int i = 0; i < cs.sendCount; i++) {
					send(Write, buf, size, 0);
					packets++;
					dataSent += size;
					
				}
				memset(buf, '\0', cs.dataSize);
				
			}
		}
		char buffer[50];
		sprintf_s(buffer,
			"\n\nStart Time %02d:%02d.%03d",
			stStartTime.wMinute,
			stStartTime.wSecond,
			stStartTime.wMilliseconds);
		send(Write, buffer, strlen(buffer), 0);
	} else {
		if (myfile.is_open()) {
			int server_len = sizeof(Server);
			while (!myfile.eof()) {
				myfile.read(buf, cs.dataSize);
				for (int i = 0; i < cs.sendCount; i++) {
					if (sendto(Write, buf, cs.dataSize, 0, (struct sockaddr *)&Server, server_len) == SOCKET_ERROR) {
						continue;
					}
				}
				memset(buf, '\0', cs.dataSize);
			}
			packets = 0;
			char buffer[50];
			sprintf_s(buffer,
				"\n\nStart Time %02d:%02d.%03d",
				stStartTime.wMinute,
				stStartTime.wSecond,
				stStartTime.wMilliseconds);
			sendto(Write, buffer, strlen(buffer), 0, (struct sockaddr *)&Server, server_len);
			
			for (int i = 0; i < 3; i++) {
				Sleep(1);
				if (sendto(Write, "t:9}m[\0", 7, 0, (struct sockaddr *)&Server, server_len)) {
					continue;
				}
			}
		}
	}
	free(buf);
	myfile.close();
	Sleep(1);
	closesocket(Write);
	connected = false;
}


