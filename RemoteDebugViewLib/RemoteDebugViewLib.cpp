#include <WinSock2.h>
#include <windows.h>

#include <stdio.h>


#define DWBUF_SIZE			4096
#define TIMEOUT_INTERVAL	100
// I doubled the buffer size because
// i'm lazy
#define PRINTBUF_SIZE		8192

#define LISTEN_PORT			3232




WCHAR wcBufferName[]	= L"DBWIN_BUFFER";
WCHAR wcBufferReady[]	= L"DBWIN_BUFFER_READY";
WCHAR wcDataReady[]		= L"DBWIN_DATA_READY";

// Referencing: http://unixwiz.net/techtips/outputdebugstring.html
struct DBWIN_BUFFER {
	DWORD	dwProcessId;
	BYTE	data[DWBUF_SIZE - sizeof(DWORD)];
};

BOOL
WriteDebugToSocket
(
	SOCKET sSocket
)
{
	BOOL bRetVal						= FALSE;
	CHAR printBuffer[PRINTBUF_SIZE]		= { 0 };
	DBWIN_BUFFER* pBuffer				= NULL;
	DWORD dwStatus						= 0;
	HANDLE hDataReady					= NULL;
	HANDLE hBufferReady					= NULL;
	HANDLE hSharedMemory				= NULL;

	// Initialize the handles and the shared memory section
	hBufferReady = OpenEvent
	(
		EVENT_ALL_ACCESS,
		FALSE,
		wcBufferReady
	);

	if (NULL == hBufferReady)
	{
		hBufferReady = CreateEvent
		(
			NULL,
			FALSE,
			TRUE,
			wcBufferReady
		);
		if (NULL == hBufferReady)
		{
			goto exit;
		}
	}

	hDataReady = OpenEvent
	(
		SYNCHRONIZE,
		FALSE,
		wcDataReady
	);

	if (NULL == hDataReady)
	{
		hDataReady = CreateEvent
		(
			NULL,
			FALSE,
			FALSE,
			wcDataReady
		);
		if (NULL == hDataReady)
		{
			goto exit;
		}
	}

	hSharedMemory = OpenFileMapping
	(
		FILE_MAP_READ,
		FALSE,
		wcBufferName
	);

	if (NULL == hSharedMemory)
	{
		hSharedMemory = CreateFileMapping
		(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(DBWIN_BUFFER),
			wcBufferName
		);
		if (NULL == hSharedMemory)
		{
			goto exit;
		}
	}

	pBuffer = (DBWIN_BUFFER*)MapViewOfFile
	(
		hSharedMemory,
		SECTION_MAP_READ,
		0,
		0,
		0
	);

	if (NULL == pBuffer)
	{
		goto exit;
	}

	while (true)
	{
		dwStatus = 0;
		dwStatus = WaitForSingleObject
		(
			hDataReady,
			TIMEOUT_INTERVAL
		);

		if (WAIT_OBJECT_0 == dwStatus)
		{
			SYSTEMTIME time = { 0 };
			GetSystemTime
			(
				&time
			);

			snprintf
			(
				printBuffer,
				8192,
				"[%04d/%02d/%02d %02d:%02d:%02d] (%d) : %s",
				time.wYear,
				time.wMonth,
				time.wDay,
				time.wHour,
				time.wMinute,
				time.wSecond,
				pBuffer->dwProcessId,
				pBuffer->data
			);

			dwStatus = send
			(
				sSocket,
				printBuffer,
				strnlen(printBuffer, PRINTBUF_SIZE),
				0
			);

			SetEvent
			(
				hBufferReady
			);

			if (SOCKET_ERROR == dwStatus)
			{
				goto exit;
			}
		}
		else if (WAIT_FAILED == dwStatus)
		{
			goto exit;
		}
	}


exit:

	if (NULL != pBuffer)
	{
		UnmapViewOfFile
		(
			pBuffer
		);
		pBuffer = 0;
	}

	if (NULL != hSharedMemory)
	{
		CloseHandle
		(
			hSharedMemory
		);
		hSharedMemory = NULL;
	}

	if (NULL != hDataReady)
	{
		CloseHandle
		(
			hDataReady
		);
		hDataReady = NULL;
	}

	if (NULL != hBufferReady)
	{
		CloseHandle
		(
			hBufferReady
		);
		hBufferReady = NULL;
	}

	return bRetVal;
}


BOOL
ListenForConnections
(
	WSADATA wsaData
)
{
	BOOL bRetVal = FALSE;
	DWORD dwStatus = 0;
	SOCKADDR_IN addr = { 0 };
	SOCKET sClient = INVALID_SOCKET;
	SOCKET sListen = INVALID_SOCKET;

	if (0 != dwStatus)
	{
		goto exit;
	}

	sListen = WSASocket
	(
		AF_INET,
		SOCK_STREAM,
		0,
		NULL,
		0,
		0
	);

	if (INVALID_SOCKET == sListen)
	{
		goto exit;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons(LISTEN_PORT);

	dwStatus = bind
	(
		sListen,
		(PSOCKADDR)&addr,
		sizeof(addr)
	);

	if (SOCKET_ERROR == dwStatus)
	{
		goto exit;
	}

	dwStatus = listen
	(
		sListen,
		SOMAXCONN
	);

	if (SOCKET_ERROR == dwStatus)
	{
		goto exit;
	}

	sClient = accept
	(
		sListen,
		NULL,
		NULL
	);

	if (INVALID_SOCKET == sClient)
	{
		goto exit;
	}

	closesocket
	(
		sListen
	);

	WriteDebugToSocket
	(
		sClient
	);

exit:

	closesocket
	(
		sClient
	);

	return bRetVal;
}

BOOL
Run()
{
	BOOL bRetVal			= FALSE;
	DWORD dwStatus			= 0;
	HANDLE hKillEvent		= NULL;
	WSADATA wsaData			= { 0 };

	CreateEvent
	(
		NULL,
		TRUE,
		FALSE,
		L"dbgview"
	);

	dwStatus = WSAStartup
	(
		MAKEWORD(2, 2),
		&wsaData
	);

	// Sit in a loop and wait for something to connect
	// in. If the kill event is set, break out and stop
	while (TRUE)
	{
		ListenForConnections
		(
			wsaData
		);
		
		dwStatus = WaitForSingleObject
		(
			hKillEvent,
			0
		);

		if (WAIT_OBJECT_0 == dwStatus)
		{
			bRetVal = TRUE;
			break;
		}
	}

	WSACleanup();

	return bRetVal;
}