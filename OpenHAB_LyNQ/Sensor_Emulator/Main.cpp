#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include "Main.h"
#pragma comment(lib,"ws2_32.lib")
using namespace std;


SOCKET Socket;
int iNumberSensor = 2;
BOOL bRun = TRUE;
char strPathConfigOpenHAB[MAX_PATH];
char strMQTTName[MAX_PATH];
ITEM item[100];

int main(){	
	WSADATA wsaData;
	struct hostent *host;
	SOCKADDR_IN SockAddr;
	HANDLE *hThread;	
	time_t t;
	HANDLE hThreadStop;		

	if (!readConfig())
	{		
		return 0;
	}

	printf_s("Do you want create .sitemap and .items file(y\\n)\n");
	if (toupper(_getch()) == 'Y')
	{
		createFileSitemap();
		createFileItem();
	}
	

	srand((unsigned)time(&t));
	char strRequest[MAX_PATH];

	hThread = (HANDLE*)malloc(iNumberSensor*sizeof(HANDLE));

	if (!hThread)
	{
		printf_s("Can't malloc mem hThread\n");
		return 1;
	}

	//initiate struct
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf_s("WSAStartup failed.\n");
		free(hThread);		
		system("pause");
		return 1;
	}

	//open socket
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//get address of the host
	host = gethostbyname(HOST);	
	SockAddr.sin_port = htons(PORT);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	//connect to server
	printf_s("Connecting...\n");
	if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0){
		printf_s("Could not connect\n");
		system("pause");
		free(hThread);		
		return 1;
	}
	printf_s("Connected.\n");
	
	for (int i = 0; i < iNumberSensor; i++)
	{		
		hThread[i] = CreateThread(NULL,
			0,
			threadSendCommand,
			&item[i],
			0,
			NULL
			);

		if (!hThread[i])
		{
			for (int j = 0; j < i; j++)
				CloseHandle(hThread[j]);
			printf_s("Fail to create thread %d\n", i);
			goto RET;
		}
	}

	hThreadStop = CreateThread(NULL, 0, threadStop, NULL, 0, NULL);

	if (hThreadStop)
	{
		WaitForMultipleObjects(iNumberSensor, hThread, TRUE, INFINITE);
		CloseHandle(hThreadStop);	
	}
	
	for (int i = 0; i < iNumberSensor; i++)
	{
		if (hThread[i])
		CloseHandle(hThread[i]);		
	}
	sprintf_s(strRequest, GET_FORMAT, "/", HOST, PORT, CLOSE_CONNECTION);
	send(Socket, strRequest, strlen(strRequest), 0);
RET:
	closesocket(Socket);
	WSACleanup();
	free(hThread);	
	system("pause");
	return 0;
}

BOOL readConfig()
{	
	FILE *f;			
	
	fopen_s(&f, FILE_CONFIG, "rt");	
	if (!f)
	{
		printf_s("Can't open file config\n");
		return FALSE;
	}

	readLineInFile(f, strPathConfigOpenHAB, MAX_PATH);
	readLineInFile(f, strMQTTName, MAX_PATH);
	fscanf_s(f, "%d", &iNumberSensor);
	for (int i = 0; i < iNumberSensor; i++)
	{
		fscanf_s(f, "%d", &item[i].iType);
		fscanf_s(f, "%d", &item[i].iTime);
		if (item[i].iType == 1)
		{
			sprintf_s(item[i].strLabel, "%s%d", TEMP_SENSOR, i);
		}
		else 
			sprintf_s(item[i].strLabel, "%s%d", HUM_SENSOR, i);
		sprintf_s(item[i].strName, "%s%d", PREFIX_NAME, i);
	}

	fclose(f);
	return TRUE;
}

void readLineInFile(FILE *f, char *str, int iSize)
{
	char c;
	int count = 0;
	
	memset(str, 0, iSize);
	while (true)
	{
		c = fgetc(f);
		if (c != '\n' && c != EOF)
		{
			str[count] = c;
			count ++ ;
		}			
		else
			break;
	}	
	
}

void getElementInLine(char *strLine, char *strOut, int iPos)
{
	int iCount = 0;
	int i = 0;
	while (iPos > 0)
	{
		if (strLine[iCount] == ' ')
			iPos--;
		if (strLine[iCount] == '\0')
			break;
		iCount++;		
	}
	while (strLine[iCount]!= ' ' && strLine[iCount] != '\0')
	{
		strOut[i] = strLine[iCount];
		i++;
	}
	strOut[i] = '\0';
}

void standardizeLine(char *strLine, char *strOut)
{	
	int iCountOut = 0;	
	BOOL bSpace = TRUE;

	for (int i = 0; strLine[i] != NULL; i++)
	{
		if (!bSpace || isSpace(strLine[i]))
		{
			bSpace = isSpace(strLine[i]);
			if (bSpace)
				strOut[iCountOut] = ' ';
			else 
				strOut[iCountOut] = strLine[i];
			iCountOut++;
		}
	}

	strOut[iCountOut] = '\0';
}

BOOL isSpace(char c)
{
	if (c == ' ' || c == '\t')
		return TRUE;
	return FALSE;
}

DWORD WINAPI threadSendCommand(LPVOID lpParam)
{
	char strRequest[MAX_PATH];
	char strBuffer[10000];
	int uDataLength;
	char strCommand[MAX_PATH];
	int iState;
	int iTime;
	int iType;
	char * strName;

	iTime = ((ITEM*)lpParam)->iTime;
	strName = ((ITEM*)lpParam)->strName;
	iType = ((ITEM*)lpParam)->iType;

	iState = rand() % 25 + 8;

	if (iType == 2)
		iState += 50;

	memset(strBuffer, 0, 10000);
	do{
		Sleep(iTime);
		iState = generateNumber(iState, iType);
		sprintf_s(strCommand, SET_STATE_FORMAT, strName, iState);
		sprintf_s(strRequest, GET_FORMAT, strCommand, HOST, PORT, "");
		send(Socket, strRequest, strlen(strRequest), 0);
		uDataLength = recv(Socket, strBuffer, 10000, 0);
		printf_s("\n_____________________________________________________________________________\n");
		printf_s("%s\n", strName);
		printf_s("%s\n", strBuffer);
	} while (uDataLength > 0 && bRun);
	return 0;
}

DWORD WINAPI threadStop(LPVOID lpParam)
{
	UNREFERENCED_PARAMETER(lpParam);
	char c = 0;
	while (true)
	{
		c = _getch();
		c = toupper(c);
		if (c == 'S')
		{
			bRun = FALSE;
			printf_s("Process will stop in several seconds\n");
			return 0;
		}
	}
}

void createFileSitemap()
{
	char strBuffer[HUGE_SIZE];	
	char strItemSiteMap[MAX_PATH];	
	char strPath[MAX_PATH];
	FILE *f;

	memset(strBuffer, 0, HUGE_SIZE);
	sprintf_s(strPath, "%s%s", strPathConfigOpenHAB, FILE_SITE_MAP);

	strcat_s(strBuffer, "sitemap Test label=\"Menu\"\n{\nFrame label=\"MAIN\"\n{");
	for (int i = 0; i < iNumberSensor; i++)
	{
		strcat_s(strBuffer, "\n");		
		sprintf_s(strItemSiteMap, SITE_MAP_FORMAT, "Text", item[i].strName);
		strcat_s(strBuffer, strItemSiteMap);
	}
	strcat_s(strBuffer, "\n}\n}");
	fopen_s(&f, strPath, "wt");
	if (f != NULL)
	{
		fprintf_s(f, "%s", strBuffer);
		fclose(f);
	}
	
}

void createFileItem()
{
	char strBuffer[HUGE_SIZE];
	char strItemItem[MAX_PATH];	
	char strPath[MAX_PATH];
	FILE *f;

	memset(strBuffer, 0, HUGE_SIZE);
	sprintf_s(strPath, "%s%s", strPathConfigOpenHAB, FILE_ITEM);
	
	for (int i = 0; i < iNumberSensor; i++)
	{		
		if (item[i].iType == 1)
			sprintf_s(strItemItem, ITEM_FORMAT, "Number", item[i].strName, item[i].strLabel, "%d °C", ICON_TEMP, strMQTTName, item[i].strName, strMQTTName, item[i].strName);
		else
			sprintf_s(strItemItem, ITEM_FORMAT, "Number", item[i].strName, item[i].strLabel, "%d %%", ICON_HUM, strMQTTName, item[i].strName, strMQTTName, item[i].strName);
		strcat_s(strBuffer, strItemItem);
		strcat_s(strBuffer, "\n");
	}
	
	fopen_s(&f, strPath, "wt");
	if (f)
	{
		fprintf_s(f, "%s", strBuffer);
		fclose(f);
	}	
}

int generateNumber(int iInitValue, int iType)
{
	int iState;
	int iResult;
	iState = rand() % 3 - 1;
	iResult =  iInitValue + iState * (rand() % 3);
	if (iType == 1)
	{
		if (iResult > 50)
			iResult = 49;
		if (iResult < 5)
			iResult = 6;
	}
	
	if (iType == 2)
	{
		if (iResult > 100)
			iResult = 99;
		if (iResult < 45)
			iResult = 46;
	}
	return iResult;
}
