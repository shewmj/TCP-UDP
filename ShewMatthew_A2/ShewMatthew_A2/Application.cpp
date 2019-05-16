/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	Main.c -    A windows program, which will be able to generate
--								TCP and UDP datagrams and transfer the data using 
--								the TCP/IP protocol suite between two Windows workstations. 
--
--	PROGRAM:		ShewMatthew_Assignment2.exe
--
--	FUNCTIONS:
--					LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
--
--	DATE:			February 12, 2018
--
--	REVISIONS:		(Date and Description)
--
--	DESIGNERS:		Matthew Shew
--
--	PROGRAMMER:		Matthew Shew
--
--	NOTES:
--      This program will be able to generate TCP and UDP datagrams and transfer the data using 
--		the TCP/IP protocol suite between two Windows workstations. 
--
---------------------------------------------------------------------------------------*/
#include "Session.h"
#include "Application.h"
#include "Physical.h"


int menuSelect = 0;
HWND hwnd;
char Name[] = "C4985_A2";
HINSTANCE hInstance;
WSADATA wsaData;
CommSettings cs;


OPENFILENAME ofn;
OVERLAPPED ol;
HANDLE fileHandle;
char filePathBuffer[MAX_FILENAME_SIZE];

string fileName;
ofstream oFile;
ifstream iFile;

SYSTEMTIME stStartTime, stEndTime;
bool started = false;

SOCKET Listen;
SOCKET Write;
SOCKADDR_IN Server;
SOCKADDR_IN Client;

bool connected = false;
int dataReceived = 0;
int packets = 0;
char buf[DATA_BUFSIZE];
/*------------------------------------------------------------------------------------------------------------------
--  FUNCTION:	WinMain
--
--  DATE:			February 12, 2018
--
--  REVISIONS:		(Date and Description)
--
--  DESIGNER:		Matthew Shew
--
--  PROGRAMMER:		Matthew Shew
--
--  INTERFACE:		int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
--						HINSTANCE hInst: the handle to the current instance of the program
--						HINSTANCE hprevInstance: the handle to the previous instance of the program
--						LPSTR lspszCmdParam: command line for the application
--						int nCmdShow: control parameter that determines how the window is shown
--
--  RETURNS:		Returns an int value based on function's success.
--						Returns 0 if the window fails to register.
--						Returns Msg.wParam otherwise.
--
--  NOTES:
--  This function is the entry point into the program, and creates the window and menu for the program.
--  It contains the core messaging loop that runs for the program's lifetime.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{

	memset(buf, '\0', DATA_BUFSIZE);
	MSG userMsg;
	WNDCLASSEX Wcl;
	hInstance = hInst;
	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style
	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background 
	Wcl.lpszClassName = Name;
	Wcl.lpszMenuName = "commandMenu"; // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	//sets memory of OPENFILEDIALOG
	memset(&ofn, 0, sizeof(ofn)); ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePathBuffer;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filePathBuffer);
	ofn.lpstrFilter = "*.TXT\0;*.txt\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ol = { 0 };

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		600, 400, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	SetupMenu(hwnd);

	while (GetMessage(&userMsg, NULL, 0, 0)) {
		TranslateMessage(&userMsg);
		DispatchMessage(&userMsg);
	}
	WSACleanup();
	return userMsg.wParam;
}


/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: WndProc
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE : LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--				HWND hwnd: The handle to the window.
--				UINT Message: .
--				WPARAM wParam: The event that has occured.
--				LPARAM lParam: .
--
--
--RETURNS : LRESULT.
--
--NOTES :
--This function is called when a user event occurs
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

	HMENU menu = GetMenu(hwnd);

	if (Message == WM_SOCKET) {
		if (WSAGETSELECTERROR(lParam)) {
			printf("Socket failed with error %d\n", WSAGETSELECTERROR(lParam));
		} else {
			switch (WSAGETSELECTEVENT(lParam)) {
				case FD_ACCEPT:
					AcceptConnection(wParam);
					break;
				case FD_READ:
					ReadFromSocket(wParam);
					break;
				case FD_WRITE:
					WriteToSocket(fileName);
					break;		
				case FD_CLOSE:
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
					break;
			}
		}
		return 0;
	}

	switch (Message) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_QUIT:
					PostQuitMessage(0);
					break;
				case IDM_IP:
					menuSelect = IDM_IP;
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_IP), hwnd, reinterpret_cast<DLGPROC>(DlgProc));
					break;
				case IDM_SEND_COUNT:
					menuSelect = IDM_SEND_COUNT;
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_SEND_COUNT), hwnd, reinterpret_cast<DLGPROC>(DlgProc));
					break;
				case IDM_PORT:
					menuSelect = IDM_PORT;
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_PORT), hwnd, reinterpret_cast<DLGPROC>(DlgProc));
					break;
				case IDM_FILE:
					if (GetOpenFileName(&ofn) == TRUE) {
						fileHandle = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
						if (fileHandle == INVALID_HANDLE_VALUE) {
							MessageBox(hwnd, "Could not open file.", "Error", NULL);
						}
						fileName = ofn.lpstrFile;
						CloseHandle(fileHandle);
					}
					break;
				case IDM_START: 
					Connect(fileName);
					break;
				default:
					UpdateSettings(wParam, menu);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CHAR:
			break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}



/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: DlgProc
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE : LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
--				HWND hwndDlg: The handle to the dialogue box.
--				UINT Message: .
--				WPARAM wParam: The event that has occured.
--				LPARAM lParam: .
--
--RETURNS : LRESULT.
--
--NOTES :
--This function is called when a user event occurs in the open dialogue box
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int num;
	char temp[30];
	memset(temp, '\0', 30);
	switch (Msg) {
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch (wParam) {
			case IDOK:
				GetDlgItemText(hWndDlg, IDC_EDIT1, temp, 30);
				switch (menuSelect) {
					case IDM_IP:
						strcpy_s(cs.ipAddress, temp);
						break;
					case IDM_SEND_COUNT:
						if (sscanf_s(temp, "%d", &num) == 1)
							cs.sendCount = num;
						else 
							MessageBox(hwnd, "send count error", "", NULL);
						break;
					case IDM_PORT:
						if (sscanf_s(temp, "%d", &num) == 1)
							cs.port = num;
						else
							MessageBox(hwnd, "port num error", "", NULL);
						break;
				}
				EndDialog(hWndDlg, 0);
				return TRUE;
			case IDCANCEL:
				EndDialog(hWndDlg, 0);
				break;
			}
			break;
	}
	return FALSE;
}


/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: SetupMenu
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE :void setupMenu(HWND hwnd)
--				HWND hwnd: the Handle for the window
--
--RETURNS : void.
--
--NOTES :
--This function is called when the menu is started up or reset
----------------------------------------------------------------------------------------------------------------------*/
void SetupMenu(HWND hwnd) {

	HMENU menu = GetMenu(hwnd);
	CheckMenuRadioItem(menu, IDM_TCP, IDM_UDP, IDM_TCP, MF_BYCOMMAND);
	CheckMenuRadioItem(menu, IDM_CLIENT, IDM_SERVER, IDM_CLIENT, MF_BYCOMMAND);
	CheckMenuRadioItem(menu, IDM_512, IDM_60K, IDM_512, MF_BYCOMMAND);
	cs.TCP_UDP = "tcp";
	cs.CLIENT_SERVER = "client";
	cs.dataSize = 512;
	cs.port = 8000;
	cs.sendCount = 1;
	strcpy_s(cs.ipAddress, "192.168.0.3");

}


/*------------------------------------------------------------------------------------------------------------------
--FUNCTION: UpdateSettings
--
--DATE : February 12, 2018
--
--REVISIONS : (Date and Description)
--
--DESIGNER : Matthew Shew
--
--PROGRAMMER : Matthew Shew
--
--INTERFACE : void updateSettings(WPARAM wParam, HEMNU hmenu)
--				WPARAM wParam: The event that has been triggered.
--				HEMNU hMenu: The current menu for the window
--
--RETURNS : void.
--
--NOTES :
--This function is called when a user event is detected that involves clicking a menu item
----------------------------------------------------------------------------------------------------------------------*/
void UpdateSettings(WPARAM wParam, HMENU menu) {
	switch (LOWORD(wParam)) {
		case IDM_TCP:
			CheckMenuRadioItem(menu, IDM_TCP, IDM_UDP, IDM_TCP, MF_BYCOMMAND);
			cs.TCP_UDP = "tcp";
			break;
		case IDM_UDP:
			CheckMenuRadioItem(menu, IDM_TCP, IDM_UDP, IDM_UDP, MF_BYCOMMAND);
			cs.TCP_UDP = "udp";
			break;
		case IDM_CLIENT:
			CheckMenuRadioItem(menu, IDM_CLIENT, IDM_SERVER, IDM_CLIENT, MF_BYCOMMAND);
			cs.CLIENT_SERVER = "client";
			break;
		case IDM_SERVER:
			CheckMenuRadioItem(menu, IDM_CLIENT, IDM_SERVER, IDM_SERVER, MF_BYCOMMAND);
			cs.CLIENT_SERVER = "server";
			break;
		case IDM_512:
			CheckMenuRadioItem(menu, IDM_512, IDM_60K, IDM_512, MF_BYCOMMAND);
			cs.dataSize = 512;
			break;
		case IDM_1024:
			CheckMenuRadioItem(menu, IDM_512, IDM_60K, IDM_1024, MF_BYCOMMAND);
			cs.dataSize = 1024;
			break;
		case IDM_4096:
			CheckMenuRadioItem(menu, IDM_512, IDM_60K, IDM_4096, MF_BYCOMMAND);
			cs.dataSize = 4096;
			break;
		case IDM_20K:
			CheckMenuRadioItem(menu, IDM_512, IDM_60K, IDM_20K, MF_BYCOMMAND);
			cs.dataSize = 20000;
			break;
		case IDM_60K:
			CheckMenuRadioItem(menu, IDM_512, IDM_60K, IDM_60K, MF_BYCOMMAND);
			cs.dataSize = 60000;
			break;
	}
	
	
}


