/*
 * Legacy Biosemi routines that call appropriate usb acquisition interface 
 * software:
 * 
 * On Windows XP or Vista32,
 *     either the Biosemi kernel mode driver
 *     or the MS 'winusb' library via bsif_winusb
 *
 * On Vista64, Win7-64, Win8-64
 *     the MS 'winusb' library via bsif_winusb
 *
 * On Linux or OSX,
 *     'libusb' library via bsif_libusb
 *
 * This routine compiles on Windows, Linux and OSX
 *
 * Copyright (C) 2003-2015 Biosemi <www.biosemi.com>
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#pragma warning( disable : 4668)	// disable specific warnings in ms headers
#pragma warning( disable : 4820)
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#pragma warning( default : 4668)	// re-enable disabled warnings
#pragma warning( default : 4820)

#include <direct.h>	// for _getcwd
#define LABVIEW_DLL_EXPORTS

#else
// linux, osx
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>		// for getpwuid
#include <pwd.h>		// for getpwuid
#endif	// _WIN32

#ifdef DOWNLOAD

#ifdef _WIN32
#pragma warning( disable : 4820)	// disable specific warnings in headers
#include "Ioctls.h"
#pragma warning( default : 4820)	// re-enable disabled warnings

#else
// linux, osx
#include "Ioctls.h"
#endif	// _WIN32

#endif	// DOWNLOAD	

#include "ringbuffer.h"

#include "bsif.h"		// biosemi interface routines

//*******************************************************************
//                       system specific defines
//*******************************************************************
// use safer string functions where possible

// windows
#ifdef _WIN32
#define SNPRINTF	sprintf_s

//#define LEGACY			// for xp, remove when no longer supported
#define _CRT_SECURE_NO_WARNINGS	1	// disable deprecation warnings

#define STRICMP		_stricmp
#define STRNICMP	_strnicmp
#endif  // _WIN32

// osx
#ifdef __APPLE__
#define SNPRINTF        snprintf
#define STRICMP		strcasecmp
#define STRNICMP	strncasecmp
#endif  // __APPLE__

// linux
#ifdef __linux__
#define SNPRINTF        snprintf
#define STRICMP		strcasecmp
#define STRNICMP	strncasecmp
#endif  // __linux__

//*******************************************************************
//                         local variables
//*******************************************************************

// user controlled parameters
#ifdef DEF_LOG
static BOOL	logging=TRUE;
#else
static BOOL	logging=FALSE;
#endif	// DEF_LOG

#ifdef DEF_DEBUG
static BOOL	debugging=TRUE;
#else
static BOOL	debugging=FALSE;
#endif	// DEF_DEBUG

#ifdef DEF_SYNC
static BOOL	syncPointer=TRUE;
#else
static BOOL	syncPointer=FALSE;
#endif	// DEF_SYNC

#ifdef DEF_STRIDE_MS
static int	stride_ms=DEF_STRIDE_MS;	// msec per buffer transfer
#else
static int	stride_ms=0;		// no msec per buffer transfer
#endif	// DEF_STRIDE_MS

#ifdef DEF_STRIDE_KB
static int	stride_kb=DEF_STRIDE_KB;	// kbytes per buffer transfer
#else
static int	stride_kb=0;		// no kbytes per buffer transfer
#endif	// DEF_STRIDE_KB

// debug variables
static char	logFilename[4096];

// socket listen or connect variables
static char	connectNode[200]={'1','2','7','.','0','.','0','.','1','\0'};
static char	connectPort[20]={'3','1','1','3','\0'};
static char	listenPort[20]={'3','1','1','3','\0'};

// misc other variables
//static const char	*version="2015-05-20";
//static const char	*version="2015-06-03";	// READ_POINTER_WITH_CYCLE added
//static const char	*version="2015-06-17";	// GET_NUM_CHANNELS_USB added
static const char	*version="2015-08-13";	// entries set node, ports, get hw param added
static FILE	*logfp=NULL;
static BOOL	ini=FALSE;	// TRUE after .ini file has been read
static int	driverFound=0;	// 0=none, 1=BioSemi windows kernel, 3=other, 4=socket
static BOOL	useUsbLib=FALSE;
static BOOL	useSocket=FALSE;
static char	iniFilenamePrefix[4096];

static int	numChannelsUsb;
static PCHAR	ringBuffer;

//*******************************************************************
//                           local routines
//*******************************************************************

static PCHAR LINKAGE
//******************
GetTimeStamp()
//******************
	{
	// get current time
	time_t curtime = time (NULL);

	// convert it to local time
	struct tm *loctime = localtime (&curtime);

	return asctime(loctime);
	}


static VOID LINKAGE
//******************
ProcessIniFile()
//******************
	{
	char	iniFilename[4096];
	char	currentDirectory[4096];
	char	homeDirectory[4096];

	/*
	 * look for .ini file first in current directory, then in home directory
	 *
         * On Windows:
	 *
	 * If cwd is "?:\Windows\" ('?' is drive letter), use either "HOMEDRIVE" + "HOMEPATH" or
         * "\TEMP as log file prefix; otherwise don't use a prefix
	 *
	 * Note: This is because, when used with LabView Workbench on Windows,  cwd
	 *	becomes "Windows\System32"; when used with a Labview generated runtime .exe,
	 *	cmd is the .exe's directory
	 *
	 * On OSX and perhaps Linux:
	 *
	 * When this routine is called by the LabView workbench, the "current
	 * working directory" becomes "/", the root directory.
	 *
	 * To avoid putting the .ini and .log files in the root directory,
	 * if "getcwd" returns "/", get the environment variable "HOME" and use it
	 * as a prefix for the .ini and .log file names.
	 *
	 */

	// determine current and home directories

#ifdef _WIN32
	if (_getcwd(currentDirectory, sizeof(currentDirectory)) == NULL)
		currentDirectory[0] = 0;
	if (_stricmp(currentDirectory+1, ":\\Windows\\") == 0)	// +1 to skip the drive letter
		currentDirectory[0] = 0;

#ifdef LEGACY	// nested within _WIN32
	char	*cp;
	if ((cp=getenv("HOMEDRIVE")) != NULL)
		strcpy(homeDirectory, cp);
	else
		homeDirectory[0] = 0;

	if ((cp=getenv("HOMEPATH")) != NULL)
		strcat(homeDirectory, cp);
	else
		strcat(homeDirectory, "\\TEMP");
#else
	size_t	envLength;
	getenv_s(&envLength, homeDirectory, sizeof(homeDirectory), "HOMEDRIVE");
	if (envLength == 0)
		homeDirectory[0] = 0;;

	getenv_s(&envLength, homeDirectory+strlen(homeDirectory),
		sizeof(homeDirectory)-strlen(homeDirectory), "HOMEPATH");
	if (envLength == 0)
		strcat_s(homeDirectory, sizeof(homeDirectory), "\\TEMP");
	else
		homeDirectory[0] = 0;
#endif	// LEGACY	// nest within _WIN32

#else	// linux, osx
	if (getcwd(currentDirectory, sizeof(currentDirectory)) == NULL)
		currentDirectory[0] = 0;
	if (strcmp(currentDirectory, "/") == 0)
		currentDirectory[0] = 0;
	
	char *cp = getenv("HOME");
	if (cp == NULL)
		{
		struct passwd *pw = getpwuid(getuid());
		if (pw != NULL)
			cp = pw->pw_dir;
		}

	if (cp != NULL)
#if defined( LEGACY ) || defined( __linux__ )	// nested within not _WIN32
		strcpy(homeDirectory, cp);
#else	// osx
		strlcpy(homeDirectory, cp, sizeof(homeDirectory));
#endif  // defined( LEGACY ) || defined( __linux__ )		// nested within not _WIN32
	else
		homeDirectory[0] = 0;
#endif	// _WIN32

	// look for .ini file first in current directory; if not found try home directory

#if defined( LEGACY ) || defined( __linux__ )
	strcpy(iniFilenamePrefix, currentDirectory);
	strcpy(iniFilename, currentDirectory);
	if (iniFilename[0] == 0)
		strcpy(iniFilename, "ringbuffer.ini");
	else
#ifdef _WIN32
		strcat(iniFilename, "\\ringbuffer.ini");
#else	// __linux__
		strcat(iniFilename, "/ringbuffer.ini");
#endif	// _WIN32
#elif defined _WIN32
	strcpy_s(iniFilenamePrefix, sizeof(iniFilenamePrefix), currentDirectory);
	strcpy_s(iniFilename, sizeof(iniFilename), currentDirectory);
	if (iniFilename[0] == 0)
		strcpy_s(iniFilename, sizeof(iniFilename), "ringbuffer.ini");
	else
		strcat_s(iniFilename, sizeof(iniFilename), "\\ringbuffer.ini");
#else	// osx
	strlcpy(iniFilenamePrefix, currentDirectory, sizeof(iniFilenamePrefix));
	strlcpy(iniFilename, currentDirectory, sizeof(iniFilename));
	if (iniFilename[0] == 0)
		strlcpy(iniFilename, "ringbuffer.ini", sizeof(iniFilename));
	else
		strlcat(iniFilename, "/ringbuffer.ini", sizeof(iniFilename)-strlen(iniFilename)-1);
#endif  // defined( LEGACY ) || defined( __linux__ )
//fprintf(stderr, "trying current directory: %s\n", iniFilename);
	FILE *cfgfp = fopen(iniFilename, "r");
	if ((cfgfp == (FILE *)NULL) && (homeDirectory[0] != 0))
		{

		// .ini not found in currentDirectory
		// try home directory

#if defined( LEGACY ) || defined( __linux__ )
		strcpy(iniFilenamePrefix, homeDirectory);
		strcpy(iniFilename, homeDirectory);
#ifdef _WIN32
		strcat(iniFilename, "\\ringbuffer.ini");
#else	// __linux__
		strcat(iniFilename, "/ringbuffer.ini");
#endif	// _WIN32

#elif defined _WIN32
		strcpy_s(iniFilenamePrefix, sizeof(iniFilenamePrefix), homeDirectory);
		strcpy_s(iniFilename, sizeof(iniFilename), homeDirectory);
		strcat_s(iniFilename, sizeof(iniFilename), "\\ringbuffer.ini");
#else	// osx
		strlcpy(iniFilenamePrefix, homeDirectory, sizeof(iniFilenamePrefix));
		strlcpy(iniFilename, homeDirectory, sizeof(iniFilename));
		strlcat(iniFilename, "/ringbuffer.ini", sizeof(iniFilename)-strlen(iniFilename)-1);
#endif  // defined( LEGACY ) || defined( __linux__ )

//fprintf(stderr, "trying home directory: %s\n", iniFilename);
		cfgfp = fopen(iniFilename, "r");
		}

	if (cfgfp == (FILE *)NULL)
		{
		fputs("No 'ringbuffer.ini' file found!\n", stderr);
		iniFilenamePrefix[0] = 0;
		}
	else
		{
		fprintf(stderr, "Using ini file: '%s'\n", iniFilename);

		// process the .ini file
		char	aline[80];
		debugging = logging = syncPointer = FALSE;
		for (;;)
			if (fgets((char *)&aline, sizeof(aline), cfgfp) == (char *)NULL)
				break;
			else
				{
				int len = (int)strlen(aline);
				if (aline[len-1] == '\n')
					aline[--len] = 0;	// drop lf
				if (aline[len-1] == '\r')
					aline[--len] = 0;	// drop cr
				if ((aline[0] == ';') || (aline[0] == '#'))
					continue;

				// convert each tab to a single blank
				for (int i=0; i<len; i++)
					if (aline[i] == '\t')
						aline[i] = ' ';

				if (STRICMP(aline, "debug") == 0)
					debugging = logging = TRUE;
				else if (STRICMP(aline, "log") == 0)
					logging = TRUE;
				else if (STRICMP(aline, "sync") == 0)
					syncPointer = TRUE;
				else if (STRICMP(aline, "nosync") == 0)
					syncPointer = FALSE;
				else if (STRNICMP(aline, "stride ", 7) == 0)
					{
					int msec = atoi(aline+7);
					if (msec > 0)
						stride_ms = msec;
					}
				else if (STRNICMP(aline, "stride_ms ", 10) == 0)
					{
					int msec = atoi(aline+10);
					if (msec > 0)
						stride_ms = msec;
					}
				else if (STRNICMP(aline, "stride_kb ", 10) == 0)
					{
					int kbytes = atoi(aline+10);
					if (kbytes > 0)
						stride_kb = kbytes;
					}
				else if (STRNICMP(aline, "listen_port ", 12) == 0)
					{
					// find next non blank
					size_t c0 = strspn(aline+12, " ");
					// find next blank
					char *blankPtr = strchr(aline+12+c0, ' ');
					if (blankPtr == NULL)
						blankPtr = aline+12+c0 + strlen(aline+12+c0);
					size_t c9 = (size_t)(blankPtr - (aline+12+c0));
					if (c9+1 < sizeof(listenPort))
						{
#ifdef _WIN32
#ifdef LEGACY
						strncpy(listenPort, aline+12+c0, c9);
#else
						strncpy_s(listenPort, sizeof(listenPort), aline+12+c0, c9);
#endif	// LEGACY
#else	// linux, osx
						strncpy(listenPort, aline+12+c0, c9);
#endif	// _WIN32
						listenPort[c9] = 0;
						}

					if (atoi(listenPort) <= 0)
						listenPort[0] = 0;	// disable listening
					}
				else if (STRNICMP(aline, "connect_node ", 13) == 0)
					{
					// find next non blank
					size_t c0 = strspn(aline+13, " ");
					// find next blank
					char *blankPtr = strchr(aline+13+c0, ' ');
					if (blankPtr == NULL)
						blankPtr = aline+13+c0 + strlen(aline+13+c0);
					size_t c9 = (size_t)(blankPtr - (aline+13+c0));
					if (c9+1 < sizeof(connectNode))
						{
#ifdef _WIN32
#ifdef LEGACY
						strncpy(connectNode, aline+13+c0, c9);
#else
						strncpy_s(connectNode, sizeof(connectNode), aline+13+c0, c9);
#endif	// LEGACY
#else	// linux, osx
						strncpy(connectNode, aline+13+c0, c9);
#endif	// _WIN32
						connectNode[c9] = 0;
						}
					}
				else if (STRNICMP(aline, "connect_port ", 13) == 0)
					{
					// find next non blank
					size_t c0 = strspn(aline+13, " ");
					// find next blank
					char *blankPtr = strchr(aline+13+c0, ' ');
					if (blankPtr == NULL)
						blankPtr = aline+13+c0 + strlen(aline+13+c0);
					size_t c9 = (size_t)(blankPtr - (aline+13+c0));
					if (c9+1 < sizeof(connectPort))
						{
#ifdef _WIN32
#ifdef LEGACY
						strncpy(connectPort, aline+13+c0, c9);
#else
						strncpy_s(connectPort, sizeof(connectPort), aline+13+c0, c9);
#endif	// LEGACY
#else	// linux, osx
						strncpy(connectPort, aline+13+c0, c9);
#endif	// _WIN32
	  					connectPort[c9] = 0;
						}

					if (atoi(connectPort) <= 0)
						connectPort[0] = 0;	// disable connecting
					}
				}
		fclose(cfgfp);

		if ((logging == TRUE) && (logfp == (FILE *)NULL))
			{
#if defined( LEGACY ) || defined( __linux__ )
			strcpy(logFilename, iniFilenamePrefix);
			if (logFilename[0] == 0)
				strcpy(logFilename, "ringbuffer.log");
			else
#ifdef LEGACY
				strcat(logFilename, "\\ringbuffer.log");
#else	// __linux__
				strcat(logFilename, "/ringbuffer.log");
#endif	// LEGACY

#elif defined _WIN32
			strcpy_s(logFilename, sizeof(logFilename), iniFilenamePrefix);
			if (logFilename[0] == 0)
				strcpy_s(logFilename, sizeof(logFilename), "ringbuffer.log");
			else
				strcat_s(logFilename, sizeof(logFilename), "\\ringbuffer.log");
#else	// osx
			strlcpy(logFilename, iniFilenamePrefix, sizeof(logFilename));
			if (logFilename[0] == 0)
				strlcpy(logFilename, "ringbuffer.log", sizeof(logFilename));
			else
				strlcat(logFilename, "/ringbuffer.log", sizeof(logFilename)-strlen(logFilename)-1);
#endif	// defined( LEGACY ) || defined( __linux__ )

			logfp = fopen(logFilename, "a");

			fprintf(logfp, "***************\n");
			fprintf(logfp, "Log started at: %s", GetTimeStamp());
			fprintf(logfp, "Version %s\n", version);
			fprintf(logfp, "Using %s\n", iniFilename);
			fprintf(logfp, "***************\n"),
				fclose(logfp),logfp=fopen(logFilename, "a");
			}

		else if ((logging == FALSE) && (logfp != (FILE *)NULL))
			fclose(logfp), logfp = (FILE *)NULL;
		}
	return;
	}

static int LINKAGE
//************************
determineNumChannels(PCHAR samBuffer, INT_PTR sizeInBytes, int firstSyncX)
//************************
	{
	/* determine the number of channels in the sample stream
	 *
	 * input is:
	 * - a sample buffer (samBuffer) and size (sizeInBytes)
	 * - index of the first sync word/status word pair of integers (firstSyncX)
	 *
	 * the number of channels is determined by looking for a consistent distance between
	 * sync/status word pairs throughout the buffer presented
	 *
	 * returns: number of channels or 0
	 */
		
	int	sizeInInts = ((int)sizeInBytes)/4;
	int	staticStatusBitsMask=0xae000000;
	int     staticStatusBits;       // mk1/2 bit and mode bits to be matched

	// save status bits at starting position
	staticStatusBits = ((int *)samBuffer)[firstSyncX+1] & staticStatusBitsMask;

	// find potential sync/status pairs

	// second sync/status pair must be in the first 10th of the buffer so there could
	// be at least 10 sync/status pairs in the buffer

	int secondSyncLimit = firstSyncX + (sizeInInts - firstSyncX)/10;
	int secondSyncX = firstSyncX + 2;
	int nch = 0;
	int guess = 0;
	for ( ; secondSyncX+1<secondSyncLimit; secondSyncX++)
		{
		if (((unsigned int)((int *)samBuffer)[secondSyncX] == 0xffffff00) &&
			((((int *)samBuffer)[secondSyncX+1] & staticStatusBitsMask) ==
			staticStatusBits))
			{
			// found a sync/status pair
			guess = secondSyncX - firstSyncX;

			// test if there are sync/status pairs at every 'guess' stride in the buffer
			int nextSyncX = secondSyncX + guess;
			for (; nextSyncX+1<sizeInInts; nextSyncX+=guess)
				{
				if (((unsigned int)((int *)samBuffer)[nextSyncX] != 0xffffff00) ||
					((((int *)samBuffer)[nextSyncX+1] & staticStatusBitsMask) !=
					staticStatusBits))
					break;          // no sync here
				}

			if (nextSyncX+1 < sizeInInts)
				continue;        // early exit taken
			else
				{
				nch = guess;
				break;          // all sync/status pairs present
				}
			}
		}

	return nch;
	}

//*******************************************************************
//                         global subroutines
//*******************************************************************

#ifdef _WIN32
// windows
BOOL APIENTRY DllMain( HANDLE /* hModule */, DWORD  ul_reason_for_call,
	LPVOID /* lpReserved */)
	{
	// note: parameters hModule, lpReserved not used so no names given

	switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			if (logfp != (FILE *)NULL)
			  	fprintf(logfp, "attached\n"),
					fclose(logfp),logfp=fopen(logFilename, "a");

			break;
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_DETACH:
			if (logfp != (FILE *)NULL)
				fprintf(logfp, "detached\n"),
					fclose(logfp),logfp=fopen(logFilename, "a");
			break;
		}
	return TRUE;
	}

#else
// linux

void __attribute__ ((constructor)) my_load(void);
void __attribute__ ((destructor)) my_unload(void);

// Called when the library is loaded and before dlopen() returns
void my_load(void)
	{
	// initialization code
	BSIF_MY_LOAD();		// initialize bsif module

	if (logfp != (FILE *)NULL)
	  	fprintf(logfp, "attached\n"),
			fclose(logfp),logfp=fopen(logFilename, "a");
	}

// Called when the library is unloaded and before dlclose() returns
void my_unload(void)
	{
	// clean-up code

	BSIF_MY_UNLOAD();	// cleanup bsif module

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "detached\n"),
			fclose(logfp),logfp=fopen(logFilename, "a");
	}	

#endif	// _WIN32

//************************
HANDLE LINKAGE OPEN_DRIVER_ASYNC(void)
//************************
	{// no longer async ... provided for compatibility 
	return OPEN_DRIVER();
	}

//************************
HANDLE LINKAGE OPEN_DRIVER(void)
//************************
	{
	if (ini == FALSE)
		{
		ProcessIniFile();
		ini = TRUE;
		}

	fprintf(stderr, "listen_port = '%s'\n", (listenPort[0]!=0)?listenPort:"Disabled");
	fprintf(stderr, "connect_node = '%s'\n", connectNode);
	fprintf(stderr, "connect_port = '%s'\n", (connectPort[0]!=0)?connectPort:"Disabled");

	useUsbLib = FALSE;
	useSocket = FALSE;
	driverFound = 0;	// 0=none

	HANDLE	ret;

#ifdef _WIN32
	if (debugging == TRUE)
		fprintf(stderr, "open_driver called\n");

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "open_driver called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");

	// on windows, try kernel driver first
	ret = CreateFile("\\\\.\\BIOSEMI", GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (ret != INVALID_HANDLE_VALUE)
		{
		driverFound = 1;	// 1=BioSemi windows kernel

		if (debugging == TRUE)
			fprintf(stderr, "Using BioSemi windows kernel USB driver\n");

		if (logfp != (FILE *)NULL)
			fprintf(logfp, "Using BioSemi windows kernel USB driver\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");
		return ret;
		}
#endif	// _WIN32
	BSIF_SET_LOG(logging);

	BSIF_SET_DEBUG(debugging);

	ret = BSIF_OPEN_DRIVER();
	if (ret != 0)
		{
		driverFound = 3;	// 3=winusb(windows) or libusb(linux,osx)
		useUsbLib = TRUE;

		if (debugging == TRUE)
			fprintf(stderr, "Using bsif_xxx driver\n");
		if (logfp != (FILE *)NULL)
			fprintf(logfp, "Using bsif_xxx driver\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");

		BSIF_SET_SYNC(syncPointer);
		if (stride_kb > 0)
			BSIF_SET_STRIDE_KB(stride_kb);
		else if (stride_ms > 0)
			BSIF_SET_STRIDE_MS(stride_ms);

		if (listenPort[0] != 0)
			{
			BSIF_TAP_SET_LOG(logging, iniFilenamePrefix);
			BSIF_TAP_SET_DEBUG(debugging, iniFilenamePrefix);
			BSIF_TAP_SET_LISTEN_PORT(listenPort);

			BSIF_TAP_OPEN_DRIVER();
			}
		}
	else if ((connectNode[0] != 0) && (connectPort[0] != 0))
		{
		driverFound = 4;	// 4=socket
		useSocket = TRUE;

		if (debugging == TRUE)
			fprintf(stderr, "Using bsif_socket driver\n");
		if (logfp != (FILE *)NULL)
			fprintf(logfp, "Using bsif_socket driver\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");

		BSIF_INJECT_SET_LOG(logging, iniFilenamePrefix);
		BSIF_INJECT_SET_DEBUG(debugging, iniFilenamePrefix);
		BSIF_INJECT_SET_SYNC(syncPointer);
		BSIF_INJECT_SET_CONNECT_NODE_PORT(connectNode, connectPort);
		if (stride_ms > 0)
			BSIF_INJECT_SET_STRIDE_MS(stride_ms);

		ret = BSIF_INJECT_OPEN_DRIVER();
		}

	return ret;
	}

//************************
BOOL LINKAGE READ_MULTIPLE_SWEEPS(HANDLE hdevice,PCHAR data,INT_PTR nBytesToRead)
//************************
	{
	numChannelsUsb = 0;
	ringBuffer = data;

	if (useUsbLib == TRUE)
		{
//		return BSIF_READ_MULTIPLE_SWEEPS(hdevice, data, nBytesToRead);
		BOOL r = BSIF_READ_MULTIPLE_SWEEPS(hdevice, data, nBytesToRead);
		if (listenPort[0] != 0)
			BSIF_TAP_READ_MULTIPLE_SWEEPS(hdevice, data, nBytesToRead);
		return r;
		}

	else if (useSocket == TRUE)
		return BSIF_INJECT_READ_MULTIPLE_SWEEPS(hdevice, data, nBytesToRead);


#ifdef _WIN32
	if (debugging == TRUE)
		fprintf(stderr, "read_multiple_sweeps called, buffer size = %d\n",
			nBytesToRead);

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "read_multiple_sweeps called, buffer size = %d\n",
			nBytesToRead),
			fclose(logfp),logfp=fopen(logFilename,"a");

	OVERLAPPED gOverlapped;
	gOverlapped.Offset = 0;
	gOverlapped.OffsetHigh = 0;	

	gOverlapped.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

	DWORD nBytesRead;
	BOOL ret;

	ret = ReadFile(hdevice, data, (DWORD)nBytesToRead, &nBytesRead,
		&gOverlapped);
	
	return ret;
#else
	return FALSE;
#endif	// _WIN32
	}

//************************
BOOL LINKAGE WRITE_SINGLE_SWEEP(HANDLE hdevice,PCHAR data,DWORD nBytesToWrite)
//************************
	{
	if ((useUsbLib == TRUE) || (useSocket == TRUE))
		{
		fprintf(stderr, "*** write_single_sweep not implemented! ***\n");

		if (logfp != (FILE *)NULL)
			fprintf(logfp, "*** write_single_sweep not implemented! ***\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");

		return FALSE;
		}

#ifdef _WIN32
	if (debugging == TRUE)	
		fprintf(stderr, "write_single_sweep called\n");
	if (logfp != (FILE *)NULL)
		fprintf(logfp, "write_single_sweep called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");

	DWORD nBytesWritten;
	return WriteFile(hdevice, data, nBytesToWrite , &nBytesWritten ,NULL);
#else
	return FALSE;
#endif	// _WIN32
	}

//************************
BOOL LINKAGE WRITE_SINGLE_SWEEP_ASYNC(HANDLE hdevice,PCHAR data,DWORD nBytesToWrite)
//************************
	{
	if ((useUsbLib == TRUE) || (useSocket == TRUE))
		{
		fprintf(stderr, "*** write_single_sweep_async not implemented! ***\n");

		if (logfp != (FILE *)NULL)
			fprintf(logfp, "*** write_single_sweep_async not implemented! ***\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");

		return FALSE;
		}

#ifdef _WIN32
	if (debugging == TRUE)	
		fprintf(stderr, "write_single_sweep_async called\n");

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "write_single_sweep_async called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");

	DWORD nBytesWritten;
	OVERLAPPED gOverlapped;
	gOverlapped.Offset = 0;
	gOverlapped.OffsetHigh = 0;	

	gOverlapped.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

	return WriteFile(hdevice, data, nBytesToWrite , &nBytesWritten ,&gOverlapped);
#else
	return FALSE;
#endif	// _WIN32
	}

//************************
BOOL LINKAGE READ_POINTER(HANDLE hdevice, PINT_PTR pointer)
//************************
	{
	if (useUsbLib == TRUE)
		{
//		return BSIF_READ_POINTER(hdevice, pointer);
		BOOL r = BSIF_READ_POINTER(hdevice, pointer);
		if (listenPort[0] != 0)
			{
			BSIF_TAP_READ_POINTER(hdevice, pointer);
			}

		if ((numChannelsUsb == 0) && (*pointer > 0))
			numChannelsUsb = determineNumChannels(ringBuffer, *pointer, 0);

		return r;
		}

	else if (useSocket == TRUE)
		{
		BOOL r = BSIF_INJECT_READ_POINTER(hdevice, pointer);

		if ((numChannelsUsb == 0) && (*pointer > 0))
			numChannelsUsb = determineNumChannels(ringBuffer, *pointer, 0);

		return r;
		}

#ifdef _WIN32
	// windows kernel driver

//	BOOL	ret;

	if (debugging == TRUE)
		fprintf(stderr, "read_pointer called\n"),
			fprintf(logfp, "read_pointer called\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");
//	DWORD junk;

//	ret = DeviceIoControl(hdevice,IOCTL_BUFFER_POINTER , NULL, 0, pointer,
//		sizeof(INT_PTR), &junk, NULL);
	if (debugging == TRUE)
		fprintf(stderr, "read_pointer returns %d\n", *pointer),
			fprintf(logfp, "read_pointer returns %d\n", *pointer),
				fclose(logfp),logfp=fopen(logFilename,"a");
//	return ret;
#endif	// _WIN32
	return FALSE;
	}

//************************
BOOL LINKAGE READ_POINTER_WITH_CYCLE(HANDLE hdevice, long long *cycleNum, PINT_PTR pointer)
//************************
	{
	if (useUsbLib == TRUE)
		{
		BOOL r = BSIF_READ_POINTER_WITH_CYCLE(hdevice, cycleNum, pointer);
		if (listenPort[0] != 0)
			{
			BSIF_TAP_READ_POINTER(hdevice, pointer);
			}

		if ((numChannelsUsb == 0) && (*pointer > 0))
			numChannelsUsb = determineNumChannels(ringBuffer, *pointer, 0);

		return r;
		}

	else if (useSocket == TRUE)
		{
		BOOL r = BSIF_INJECT_READ_POINTER_WITH_CYCLE(hdevice, cycleNum, pointer);

		if ((numChannelsUsb == 0) && (*pointer > 0))
			numChannelsUsb = determineNumChannels(ringBuffer, *pointer, 0);

		return r;
		}

	return FALSE;
	}

//************************
BOOL LINKAGE USB_WRITE(HANDLE hdevice, PCHAR data)
//************************
	{
	if (useUsbLib == TRUE)
		return BSIF_USB_WRITE(hdevice, data);

	else if (useSocket == TRUE)
		return BSIF_INJECT_USB_WRITE(hdevice, data);

#ifdef _WIN32
	// windows kernel driver

	if (debugging == TRUE)
		fprintf(stderr, "usb_write called, [0]=%x - %s\n",
			(int)data[0], (data[0]==0)?"disable":"enable");

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "usb_write called, [0]=%x - %s\n",
			(int)data[0], (data[0]==0)?"disable":"enable"),
			fclose(logfp),logfp=fopen(logFilename,"a");

//	DWORD junk;
//	OVERLAPPED gOverlapped;
//	gOverlapped.Offset = 0;
//	gOverlapped.OffsetHigh = 0;	

//	gOverlapped.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	
//	return DeviceIoControl(hdevice,IOCTL_USB_WRITE, NULL, 0, data, 64, &junk,NULL);
#endif	// _WIN32
	return FALSE;
	}

//************************
BOOL LINKAGE CLOSE_DRIVER_ASYNC(HANDLE hdevice)
//************************
	{// no longer async ... provided for compatibility
	return CLOSE_DRIVER(hdevice);
	}

//************************
BOOL LINKAGE CLOSE_DRIVER(HANDLE hdevice)
//************************
	{
	if (useUsbLib == TRUE)
		{
		if (listenPort[0] != 0)
			BSIF_TAP_CLOSE_DRIVER(hdevice);
		return BSIF_CLOSE_DRIVER(hdevice);
		}

	else if (useSocket == TRUE)
		return BSIF_INJECT_CLOSE_DRIVER(hdevice);

#ifdef _WIN32
	// windows kernel driver

	if (debugging == TRUE)
		fprintf(stderr, "close_driver called\n");

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "close_driver called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");

	CancelIo(hdevice);

	return CloseHandle(hdevice);
#else
	return FALSE;
#endif	// _WIN32
	}

//***********************
FLOAT LINKAGE GET_BYTES_PER_MSEC()
//***********************
        {
	if (useUsbLib == TRUE)
		return BSIF_GET_BYTES_PER_MSEC();

	else if (useSocket == TRUE)
		return BSIF_INJECT_GET_BYTES_PER_MSEC();

	else
		return 0.0;
        }

//***********************
VOID LINKAGE SET_SYNC(BOOL onOff)
//***********************
        {
	if (useUsbLib == TRUE)
		BSIF_SET_SYNC(onOff);

	else if (useSocket == TRUE)
		BSIF_INJECT_SET_SYNC(onOff);

	return;
	}

//***********************
BOOL LINKAGE GET_SYNC()
//***********************
        {
	if (useUsbLib == TRUE)
		return BSIF_GET_SYNC();

	else if (useSocket == TRUE)
		return BSIF_INJECT_GET_SYNC();

	else
		return FALSE;
        }

//***********************
VOID LINKAGE SET_STRIDE_MS(int msec)
//***********************
        {
	if (useUsbLib == TRUE)
		BSIF_SET_STRIDE_MS(msec);

	else if (useSocket == TRUE)
		BSIF_INJECT_SET_STRIDE_MS(msec);

	return;
	}
  
//***********************
VOID LINKAGE SET_STRIDE_KB(int kBytes)
//***********************
        {
	if (useUsbLib == TRUE)
		BSIF_SET_STRIDE_KB(kBytes);

	else if (useSocket == TRUE)
		BSIF_INJECT_SET_STRIDE_KB(kBytes);

	return;
	}

//***********************
INT LINKAGE GET_STRIDE()
//***********************
        {
	if (useUsbLib == TRUE)
		return BSIF_GET_STRIDE();

	else if (useSocket == TRUE)
		return BSIF_INJECT_GET_STRIDE();

	else
		return 64;
        }

//***********************
BOOL LINKAGE SET_TRANSFER_CHANNEL_REQUEST_RANGES(int numRanges, int *channelRanges)
//***********************
	{
	BOOL	result;

	result = true;

	if (useSocket == TRUE)
		result = BSIF_INJECT_SET_TRANSFER_CHANNEL_REQUEST_RANGES(numRanges, channelRanges);
			
	return result;
	}

//***********************
INT LINKAGE GET_NUM_CHANNELS_XFR()
//***********************
        {
	int	result;

	result = 0;

	if (useSocket == TRUE)
		result = BSIF_INJECT_GET_NUM_CHANNELS_XFR();

	return result;
        }

//***********************
INT LINKAGE GET_NUM_CHANNELS_USB()
//***********************
        {
	return numChannelsUsb;
        }

//***********************
INT LINKAGE GET_NUM_RESYNCS()
//***********************
        {
	int	result;

	result = 0;

	if (useSocket == TRUE)
		result = BSIF_INJECT_GET_NUM_RESYNCS();

	return result;
        }

//***********************
INT LINKAGE GET_CHANNEL_THROUGHPUT()
//***********************
        {
	int	result;

	result = 0;

	if (numChannelsUsb > 0)
		result = (int)(((double)GET_BYTES_PER_MSEC()*1000.)/(double)(numChannelsUsb*4));

	return result;
        }

//***********************
INT LINKAGE GET_REMOTE_CHANNEL_THROUGHPUT()
//***********************
        {
	int	result;

	result = 0;

	if (useSocket == TRUE)
		result = BSIF_INJECT_GET_REMOTE_CHANNEL_THROUGHPUT();

	return result;
        }

//***********************
INT LINKAGE GET_REMOTE_ERRORCODE()
//***********************
        {
	int	result;

	result = 0;

	if (useSocket == TRUE)
		result = BSIF_INJECT_GET_REMOTE_ERRORCODE();

	return result;
        }

#ifdef DOWNLOAD
#if _WIN32
static VOID LINKAGE
//************************
PrintLastErrorAsText()
//************************
	{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0,
  		(LPTSTR) &lpMsgBuf, 0, NULL);
	fprintf(stderr, "%s\n", (LPSTR)lpMsgBuf);
	if (logfp != (FILE *)NULL)
		fprintf(logfp, "%s\n", (LPSTR)lpMsgBuf),
			fclose(logfp),logfp=fopen(logFilename,"a");
	return;
	}
#endif	// _WIN32
//************************
BOOL LINKAGE DOWNLOAD_FX2(void)
//************************
	{
	if (useUsbLib == TRUE)
		{
		fprintf(stderr, "*** download_fx2 not implemented! ***\n");

		if (logfp != (FILE *)NULL)
			fprintf(logfp, "*** download_fx2 not implemented! ***\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");

		return FALSE;
		}

	BOOL result=FALSE;
#ifdef _WIN32
	if (debugging == TRUE)	
		fprintf(stderr, "download_fx2 called\n");

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "download_fx2 called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");

//	BOOL result=FALSE;
	HANDLE hdevice;
	DWORD junk;
	FILE *fp;
	PUCHAR data[MAX_FILE_SIZE];
	ANCHOR_DOWNLOAD_CONTROL downloadControl;

	downloadControl.Offset=0;

	if ((fp = fopen("C:/BiosemiCAD/loopback/firmware/USB2a.bix","rb")) != NULL)
		{
		fread(data,sizeof(unsigned char),MAX_FILE_SIZE,fp);
		fclose(fp);
				
		hdevice = CreateFile("\\\\.\\BIOSEMI", GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, OPEN_EXISTING,0 , NULL);
	
		if(hdevice != INVALID_HANDLE_VALUE)
			{
			result = DeviceIoControl(hdevice, IOCTL_DOWNLOAD_FX2,
				&downloadControl,sizeof(ANCHOR_DOWNLOAD_CONTROL),
				data, MAX_FILE_SIZE, &junk, NULL);
			CloseHandle(hdevice);
			}
		}
#endif	// _WIN32
	return result;
	}



//************************
BOOL LINKAGE RESET_FX2(BOOL RESET)
//************************
	{
	if (useUsbLib == TRUE)
		{
		fprintf(stderr, "*** reset_fx2 not implemented! ***\n");

		if (logfp != (FILE *)NULL)
			fprintf(logfp, "*** reset_fx2 not implemented! ***\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");

		return FALSE;
		}

	BOOL bResult	= FALSE;
#ifdef _WIN32

	if (debugging == TRUE)	
		fprintf(stderr, "reset_fx2 called\n");

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "reset_fx2 called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");

	int	nBytes = 0;
	
	VENDOR_REQUEST_IN myRequest;
	HANDLE hdevice;			
	myRequest.bRequest = 0xA0;
	myRequest.wValue = 0xE600; // using CPUCS.0 in FX2
	myRequest.wIndex = 0x00;
	myRequest.wLength = 0x01;
	myRequest.bData = (BYTE)RESET;
	myRequest.direction = 0x00;
				
	hdevice = CreateFile("\\\\.\\BIOSEMI", GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if(hdevice != INVALID_HANDLE_VALUE)
		{
		bResult = (BOOLEAN)DeviceIoControl (hdevice,
			IOCTL_VENDOR_REQUEST,
			&myRequest,
			sizeof(VENDOR_REQUEST_IN),
			NULL,
			0,
			(unsigned long *)&nBytes,
			NULL);

		CloseHandle(hdevice);
		}
#endif	// _WIN32
	
	return bResult;
	}
			
//************************
BOOL LINKAGE DOWNLOAD_FX2_EEPROM(void)
//************************
	{
	ProcessIniFile();

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "download_fx2_eeprom called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");

#ifndef _WIN32
// linux, osx
	BSIF_SET_LOG(logging);
	return BSIF_DOWNLOAD_FX2_EEPROM();

#else
// windows

	int i;

	WORD Total_Read;
	BOOL result=FALSE;
	HANDLE hdevice;
	DWORD junk;
	FILE *fp;
	UCHAR data[MAX_FILE_SIZE];
	ANCHOR_DOWNLOAD_CONTROL downloadControl;

	downloadControl.Offset=0;

	for(i=0;i<MAX_FILE_SIZE;i++)data[i]=0xff;

	if (((fp = fopen("USB2a.iic","rb")) != NULL) ||
		((fp = fopen("C:/BiosemiCAD/loopback/firmware/USB2a.iic","rb")) != NULL))
		{
		Total_Read=(WORD)fread(data,sizeof(unsigned char),MAX_FILE_SIZE,fp);
		fclose(fp);
		
		// try kernel driver first		
		hdevice = CreateFile("\\\\.\\BIOSEMI", GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, OPEN_EXISTING,0 , NULL);	
		if(hdevice != INVALID_HANDLE_VALUE)
			{
			if (logfp != (FILE *)NULL)
				fprintf(logfp, "Using Biosemi kernel driver\n"),
					fclose(logfp),logfp=fopen(logFilename,"a");

			result = DeviceIoControl(hdevice,
				IOCTL_DOWNLOAD_FX2_EEPROM, &downloadControl,
				sizeof(ANCHOR_DOWNLOAD_CONTROL),
				data,Total_Read, &junk,NULL);
			if (result == FALSE)
				{
				PrintLastErrorAsText();

				if (logfp != (FILE *)NULL)
					fprintf(logfp, "*** File 'USB2a.iic' NOT downloaded!!! ***\n"),
						fclose(logfp),logfp=fopen(logFilename,"a");
				}
			else
				if (logfp != (FILE *)NULL)
					fprintf(logfp, "File 'USB2a.iic' downloaded\n"),
						fclose(logfp),logfp=fopen(logFilename,"a");


			CloseHandle(hdevice);
			}
		else
			{
			// try ms winusb driver
			if (logfp != (FILE *)NULL)
				fprintf(logfp, "Using MS WinUsb driver\n"),
					fclose(logfp),logfp=fopen(logFilename,"a");

			BSIF_SET_LOG(logging);
			return BSIF_DOWNLOAD_FX2_EEPROM();
			}
		}
	else
		{
		fprintf(stderr, "*** File 'USB2a.iic' not found!!! ***\n");
		if (logfp != (FILE *)NULL)
			fprintf(logfp, "*** File 'USB2a.iic' not found!!! ***\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");
		}

	return result;
#endif	// _WIN32
	}

//************************
BOOL LINKAGE DOWNLOAD_FX2_IIC_FIRMWARE(void)
//************************
	{
	ProcessIniFile();

	if (logfp != (FILE *)NULL)
		fprintf(logfp, "download_fx2_iic_firmware called\n"),
			fclose(logfp),logfp=fopen(logFilename,"a");
#ifndef _WIN32
// linux, osx
	BSIF_SET_LOG(logging);
	return BSIF_DOWNLOAD_FX2_IIC_FIRMWARE();

#else
// windows

	BOOL result=FALSE;
	HANDLE hdevice;
	DWORD junk;
	FILE *fp;
	PUCHAR data[MAX_FILE_SIZE];
	ANCHOR_DOWNLOAD_CONTROL downloadControl;

	downloadControl.Offset=0;

	if (((fp = fopen("Vend_Ax.bix","rb")) != NULL) ||
		((fp = fopen("C:/BiosemiCAD/loopback/Vend_ax/Vend_Ax.bix","rb")) != NULL))
		{
		fread(data,sizeof(unsigned char),MAX_FILE_SIZE,fp);				
		fclose(fp);
		
		// try kernel driver first		
		hdevice = CreateFile("\\\\.\\BIOSEMI", GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	
		if(hdevice != INVALID_HANDLE_VALUE)
			{
			if (logfp != (FILE *)NULL)
				fprintf(logfp, "Using Biosemi kernel driver\n"),
					fclose(logfp),logfp=fopen(logFilename,"a");

			result = DeviceIoControl(hdevice, IOCTL_DOWNLOAD_FX2,
				&downloadControl, sizeof(ANCHOR_DOWNLOAD_CONTROL),
				data,MAX_FILE_SIZE, &junk, NULL);
			if (result == FALSE)
				{
				PrintLastErrorAsText();

				if (logfp != (FILE *)NULL)
					fprintf(logfp, "*** File 'Vend_Ax.bix' NOT downloaded!!! ***\n"),
						fclose(logfp),logfp=fopen(logFilename,"a");
				}
			else
				if (logfp != (FILE *)NULL)
					fprintf(logfp, "File 'Vend_Ax.bix' downloaded\n"),
						fclose(logfp),logfp=fopen(logFilename,"a");

			CloseHandle(hdevice);
			}
		else
			{
			// try ms winusb driver
			if (logfp != (FILE *)NULL)
				fprintf(logfp, "Using MS WinUsb driver\n"),
					fclose(logfp),logfp=fopen(logFilename,"a");

			BSIF_SET_LOG(logging);
			return BSIF_DOWNLOAD_FX2_IIC_FIRMWARE();
			}
		}
	else
		{
		if (logfp != (FILE *)NULL)
			fprintf(logfp, "*** File 'Vend_Ax.bix' not found!!! ***\n"),
				fclose(logfp),logfp=fopen(logFilename,"a");
		}

	return result;
#endif	// _WIN32
	}

#endif // DOWNLOAD
//************************
PCHAR LINKAGE GET_DRIVER_INFO(PCHAR infoBuffer, SIZE_T infoSize)
//************************
	{
	char	stringBuffer[200];

	if (driverFound == 1)
		{
#ifdef _WIN32
#ifdef LEGACY
		sprintf(stringBuffer, 
			"V%s using: BioSemi kernel driver", version);
#else
		sprintf_s(stringBuffer, sizeof(stringBuffer),
			"V%s using: BioSemi kernel driver", version);
#endif	// LEGACY
#else	// linux, osx
		snprintf(stringBuffer, sizeof(stringBuffer),
			"V%s using: BioSemi kernel driver", version);
#endif	// _WIN32
		}

	else if (driverFound == 3)
		return BSIF_GET_DRIVER_INFO(infoBuffer, infoSize);

	else if (driverFound == 4)
		return BSIF_INJECT_GET_DRIVER_INFO(infoBuffer, infoSize);
/*		{
#ifdef _WIN32
#ifdef LEGACY
		sprintf(stringBuffer, 
			"V%s using: BioSemi TCP socket driver", version);
#else
		sprintf_s(stringBuffer, sizeof(stringBuffer),
			"V%s using: BioSemi TCP socket driver", version);
#endif	// LEGACY
#else	// linux, osx
		snprintf(stringBuffer, sizeof(stringBuffer),
			"V%s using: BioSemi TCP socket driver", version);
#endif	// _WIN32
		}
*/
	else
		{
#ifdef _WIN32
#ifdef LEGACY
		sprintf(stringBuffer, 
			"V%s: No driver found!", version);
#else
		sprintf_s(stringBuffer, sizeof(stringBuffer),
			"V%s: No driver found!", version);
#endif	// LEGACY
#else	// linux, osx
		snprintf(stringBuffer, sizeof(stringBuffer),
			"V%s: No driver found!", version);
#endif	// _WIN32
		}
	
	int retlen = (int)strlen(stringBuffer);
	if (retlen+1 > (int)infoSize)
		retlen = (int)infoSize - 1;

#ifdef _WIN32
#ifdef LEGACY
	strncpy(infoBuffer, stringBuffer, (SIZE_T)retlen);
#else
	strncpy_s(infoBuffer, (SIZE_T)infoSize, stringBuffer, (SIZE_T)retlen);
#endif	// LEGACY
#else	// linux, osx
	strncpy(infoBuffer, stringBuffer, (size_t)retlen);
#endif	// _WIN32
	infoBuffer[retlen] = 0;

	return infoBuffer;
	}

//************************
PCHAR LINKAGE GET_HARDWARE_PARAMETER(INT which, PCHAR infoBuffer, SIZE_T infoSize)
//************************
	{
	return BSIF_GET_HARDWARE_PARAMETER(which, infoBuffer, infoSize);
	}

//***********************
VOID LINKAGE SET_LISTEN_PORT(const char *port)
//***********************
        {
	if (port != NULL)
		if (port[0] != 0)
			{
			size_t len = strlen(port);
			if (len+1 < sizeof(listenPort))
				{
#ifdef _WIN32
#ifdef LEGACY
				strncpy(listenPort, port, len);
#else
				strncpy_s(listenPort, sizeof(listenPort), port, len);
#endif	// LEGACY
#else	// linux, osx
				strncpy(listenPort, port, len);
#endif	// _WIN32
				listenPort[len] = 0;
				
				if (listenPort[0] != 0)
					BSIF_TAP_SET_LISTEN_PORT(listenPort);
				}
				
			if (atoi(listenPort) <= 0)
				listenPort[0] = 0;	// disable listening
			}

	return;
	}

//***********************
VOID LINKAGE SET_CONNECT_NODE_PORT(const char *node, const char *port)
//***********************
        {
	if (node != NULL)
		if (node[0] != 0)
			{
			size_t len = strlen(node);
			if (len+1 < sizeof(connectNode))
				{
#ifdef _WIN32
#ifdef LEGACY
				strncpy(connectNode, node, len);
#else
				strncpy_s(connectNode, sizeof(connectNode), node, len);
#endif	// LEGACY
#else	// linux, osx
				strncpy(connectNode, node, len);
#endif	// _WIN32
				connectNode[len] = 0;
				
				if ((connectNode[0] != 0) && (connectPort[0] != 0))
					BSIF_INJECT_SET_CONNECT_NODE_PORT(connectNode, connectPort);
				}
			}

	if (port != NULL)
		if (port[0] != 0)
			{
			size_t len = strlen(port);
			if (len+1 < sizeof(connectPort))
				{
#ifdef _WIN32
#ifdef LEGACY
				strncpy(connectPort, port, len);
#else
				strncpy_s(connectPort, sizeof(connectPort), port, len);
#endif	// LEGACY
#else	// linux, osx
				strncpy(connectPort, port, len);
#endif	// _WIN32
				connectPort[len] = 0;
				}
				
			if (atoi(connectPort) <= 0)
				connectPort[0] = 0;	// disable connecting
				
			if ((connectNode[0] != 0) && (connectPort[0] != 0))
				BSIF_INJECT_SET_CONNECT_NODE_PORT(connectNode, connectPort);
			}
	return;
	}
