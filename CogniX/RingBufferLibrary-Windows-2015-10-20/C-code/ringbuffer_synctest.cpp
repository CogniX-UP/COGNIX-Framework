#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#pragma warning( disable : 4668)	// disable specific warnings in ms headers
#pragma warning( disable : 4820)
#include <stdio.h> 
#include <windows.h>
#pragma warning( default : 4668)	// re-enable disabled warnings
#pragma warning( default : 4820)

#include <conio.h>
#else
// linux
#include <stdio.h> 
#include <unistd.h>	// usleep
#include <errno.h>
#include <signal.h>
#endif // _WIN32

#include "ringbuffer.h"

#ifndef _WIN32
// linux
#define VOID	void
#define LPVOID	void*
#define MEM_COMMIT	0
#define MEM_RELEASE	0
#define PAGE_READWRITE	0

typedef void (*sighandler_t)(int);

static int	do_exit=0;

static int _kbhit(void)
	{
	return do_exit;
	}

static int _getch()
	{
	do_exit = 0;
	return (int)'q';
	}

static void sighandler(int signum)
	{
	printf("got signal %d\n", signum);
	do_exit = 1;
	}

static int GetLastError()
	{
	return errno;
	}

static char *VirtualAlloc(int *junk1, SIZE_T size, int junk2, int junk3)
	{
	return (char *)malloc(size);
	}

static int VirtualFree(PUCHAR junk1, SIZE_T junk2, int junk3)
	{
	free(junk1);
	return 1;
	}

static void Sleep(int msecs)
        {// sleep for msecs milliseconds
        struct timespec waitTime;

	if (msecs > 0)
		{
        	waitTime.tv_sec = msecs/1000;
        	waitTime.tv_nsec = (msecs % 1000)*1000*1000;

        	while (nanosleep(&waitTime, &waitTime)!=0)
			;
		}

        return;
        }

#endif	// not _WIN32

VOID 
PrintLastErrorAsText()
	{
#ifdef _WIN32
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0,
    		(LPTSTR) &lpMsgBuf, 0, NULL);
	printf("%s\n", (LPSTR)lpMsgBuf);
#endif // _WIN32
	return;
	}

#ifdef _WIN32
int __cdecl
#else
int 
#endif
main (int nargs, char *argv[]) 

	{
	int	nsecs;
	HANDLE	usbHandle;
	BOOL	nStatus;
	int	sec;
	char	infoBuffer[120];
	char	hwParameterBuffer[120];
	INT_PTR	ringBufferSize;
	int	plusBytes;
	int	staticStatusBitsMask=0xae000000;
	int	staticStatusBits=0;	// mk1/2 bit and mode bits in 1st buffer
	int	transferRangeStart;
	int	transferRangeEnd;
#ifndef _WIN32
        struct sigaction sigact;
 
        sigact.sa_handler = sighandler;
        sigemptyset(&sigact.sa_mask);
        sigact.sa_flags = 0;
        sigaction(SIGINT, &sigact, NULL);
        sigaction(SIGTERM, &sigact, NULL);
        sigaction(SIGQUIT, &sigact, NULL);
#endif	// not _WIN32

	printf("****************************************\n");
	printf("Parameters can be, in any order:\n");
	printf("    Time to run, in seconds:\n");
	printf("        default is 600 seconds (10 minutes)\n");
	printf("        0=unlimited\n\n");
	puts(  "    Range of channels to request from remote sample server,");
	puts(  "        must have a 'r' or 'R' prefix and");
	puts(  "        be two numbers separated by a '-'");
	puts(  "        default is all sampled channels");
	puts(  "        note: the sync and status channels are always added\n");
	printf("    Ring buffer size, in 512 byte units,\n");
	printf("        must have a 'b' or 'B' prefix:\n");
	printf("        default is B65536 for 32 MBytes\n\n");
#ifdef _WIN32
	printf("    Ring buffer extra bytes beyond the 512 multiple,\n");
	printf("        must have a '+' prefix and be a multiple of 4;\n");
	printf("        default is +0 for no extra bytes\n\n");
#endif	// _WIN32
#ifdef _WIN32
	printf("\nHit any key to quit\n");
#else
	printf("\nHit ctrl-c to quit\n");
#endif
	printf("****************************************\n");

	printf("sizeof long = %d\n", (int)sizeof(long));
	printf("sizeof size_t = %d\n\n", (int)sizeof(size_t));

	nsecs = 600;
	ringBufferSize = 65536;
	plusBytes = 0;
	transferRangeStart = 0;
	transferRangeEnd = 0;

	for (int i=1; i<nargs; i++)
		if ((argv[i][0] == 'b') || (argv[i][0] == 'B'))
			ringBufferSize = atoi(argv[i]+1);
		else if (argv[i][0] == '+')
			plusBytes = atoi(argv[i]+1);
		else if ((argv[i][0] == 'r') || (argv[i][0] == 'R'))
			{
			int dash = 2;
			for ( ; dash<6; dash++)
				if (argv[i][dash] == '-')
					break;
			if (dash < 6)
				{
				transferRangeEnd = atoi(argv[i]+dash+1);
				argv[i][dash] = 0;
				transferRangeStart = atoi(argv[i]+1);
				}	
			}
		else
			nsecs = atoi(argv[i]);

	if (plusBytes/4*4 != plusBytes)
		{
		printf("\n*** +bytes not a multiple of 4! ***\n\n");
		return 1;
		}

	sec = 0;

	// get hardware parameters being used
	for (int parNum=1; parNum<10; parNum++)
		{
		GET_HARDWARE_PARAMETER(parNum, hwParameterBuffer, sizeof(hwParameterBuffer));	
		if (hwParameterBuffer[0] == 0)
			break;
		switch (parNum)
			{
			case 1:			
				printf("Receiver chip version: '%s'\n", hwParameterBuffer);
				break;
			case 2:
				printf("Receiver FPGA version: '%s'\n", hwParameterBuffer);
				break;
			case 3:
				printf("AD-Box version: '%s'\n", hwParameterBuffer);
				break;
			default:
				printf("Hardware Parameter %d: '%s'\n", parNum, hwParameterBuffer);
			}
		}

	// get version of ringbuffer library being used	
	GET_DRIVER_INFO(infoBuffer, sizeof(infoBuffer));

	// isolate the version number
	char *cp = strchr(infoBuffer, ':');
	if (cp != (char *)NULL)
		*cp = 0;

	printf("Using RingBuffer Library version: '%s'\n\n", infoBuffer);

	if ((usbHandle=OPEN_DRIVER()) == (HANDLE)NULL)
		{
		printf("Can't open device driver!\n");
		return 1;
		}	

	/********************************
	 * allocate the large ring buffer
	 * *****************************/

	PUCHAR	samBuffer;

	INT_PTR bytes2Use = ((INT_PTR)ringBufferSize)*512 + (INT_PTR)plusBytes;
	
	printf("\nring buffer size = %d\n", (int)bytes2Use);

	if ((samBuffer=(PUCHAR)VirtualAlloc(NULL, (SIZE_T)bytes2Use, MEM_COMMIT, 
		PAGE_READWRITE)) == 0)
		{
		printf("VirtualAlloc returns 0, GetLastError = %d\n",
			GetLastError());
		PrintLastErrorAsText();
		return 0;
		}
	
	/***************************************************
	 * set transfer channel ranges, if specified by user
	 * *************************************************/

	if ((transferRangeStart > 0) && (transferRangeStart < transferRangeEnd))
		{
		// note: this program checks for presence of sync and status words
		// so channels 1 and 2 will always be included, even if not
		// specified by the user

		int numRanges;
		int transferChannelRequestRanges[4];
		int numChannelsXfr;

		transferChannelRequestRanges[0] = 1;
		if (transferRangeStart > 3)
			{
			transferChannelRequestRanges[1] = 2;
			transferChannelRequestRanges[2] = transferRangeStart; 
			transferChannelRequestRanges[3] = transferRangeEnd;
			numRanges = 2;
			numChannelsXfr = (transferChannelRequestRanges[1] - transferChannelRequestRanges[0]) + 1 +
				(transferChannelRequestRanges[3] - transferChannelRequestRanges[2]) + 1;
			}
		else
			{
			transferChannelRequestRanges[1] = transferRangeEnd; 
			numRanges = 1;
			numChannelsXfr = (transferChannelRequestRanges[1] - transferChannelRequestRanges[0]) + 1;
			}

		if (SET_TRANSFER_CHANNEL_REQUEST_RANGES(numRanges, transferChannelRequestRanges) == FALSE)
			puts("*** Invalid transfer channel range specified!");
		else
			printf("Number of transfer channel requested = %d\n", numChannelsXfr);
		}

	/**********************************************
	 * connect the ring buffer and the usb driver
	 * *******************************************/

	READ_MULTIPLE_SWEEPS(usbHandle, (PCHAR)samBuffer, bytes2Use);
	
	/************************************************
	 * start the flow of samples to the ring buffer
	 * *********************************************/

	CHAR	controlBuffer[64];
//	ULONG	bytesWritten;

	for (int i=0; i<64; i++)
		controlBuffer[i] = 0;
	controlBuffer[0] = (CHAR)(-1);

	Sleep(1000);		// msec sleep

	nStatus = USB_WRITE(usbHandle, &controlBuffer[0]);	// enable

	if (nStatus == FALSE)
		{
		// ERROR
//		printf ("usb_write for enable handshake trouble %d\n",
//			GetLastError()); 
//		PrintLastErrorAsText();
		puts("usb_write for enable handshake trouble");
		return 7;
		}

	INT_PTR	seam=0;
	INT_PTR	lastSeam=0;
	long long cycleNum=0;
	long long lastCycleNum=0;
	double	dsec;

	PUCHAR	curBuffer = samBuffer;

	INT_PTR	bytesRead;
	BOOL	first;
	int	nextSync;
	time_t	startTime;
	time_t	curTime;
	int	nch;
	int	stride;
	BOOL	pointerSync;
	int	printFreq;
	BOOL	read_p_ret;

	first=TRUE;
	nextSync=0;
	startTime=time(NULL);
	stride = 0;		// until known
	pointerSync = FALSE;	// until known
	printFreq = 10;		// every 10 buffers, if stride is 64 msec; adjusted when stride known
	infoBuffer[0] = 0;
	read_p_ret = FALSE;
	nch = 0;

	int nStatusLines = 0;
	for (int nbuffers=0;;nbuffers++)
		{
		if (pointerSync == FALSE)
#ifdef _WIN32
			if (stride > 20)
				Sleep((DWORD)((stride==0)?125:(stride>1)?(stride/2):1));
#else
			Sleep((stride==0)?125:(stride>1)?(stride/2):1);
#endif	// _WIN32

		if (nsecs > 0)
			{// time limited run
   			curTime = time(NULL);			// Get system time
   			sec = (int)difftime(curTime, startTime);
			if (sec > nsecs)
				{// run time limit reached
				printf("*** Run time limit (%d seconds) reached\n", nsecs);
				break;				// time to quit
				}
			}

		/********************************
		 * wait for a buffer
		 * *****************************/

		for (;;)
			{
			bytesRead = 0;

			// quit if a key is hit
			if (_kbhit())
				{
				_getch();
				break;
				}

			if ((read_p_ret=READ_POINTER_WITH_CYCLE(usbHandle, &cycleNum, &seam)) == FALSE)
				{
				// a single FALSE could mean re-syncing socket input
				printf("READ_POINTER returns FALSE!\n");
				if ((read_p_ret=READ_POINTER_WITH_CYCLE(usbHandle, &cycleNum, &seam)) == FALSE)
					{
					// two FALSE returns in a row means acquisition not running
					printf("READ_POINTER returns FALSE twice -> acquisition not running!\n");
					break;		// assume acquisition stopped
					}
				}

			// check that acquisition hasn't wrapped around on itself
			if ((cycleNum > lastCycleNum+1) ||
				((cycleNum == lastCycleNum+1) && (seam >= lastSeam)))
				{
				// sample overrun
				printf("ring buffer overrun!\n");
				printf("last position: cycleNum %lld, seam %d\n", lastCycleNum, (int)lastSeam);
				printf("this position: cycleNum %lld, seam %d\n", cycleNum, (int)seam);
				break;
				}
			
			if (seam != lastSeam)
				{
				bytesRead = seam - lastSeam;
				if (bytesRead < 0)
					bytesRead += bytes2Use;
				if (bytesRead > 0)
					break;
				}

			if (pointerSync == FALSE)
#ifdef _WIN32
				if (stride > 20)
					Sleep((DWORD)((stride==0)?125:(stride>1)?(stride/2):1));
#else
				Sleep((stride==0)?125:(stride>1)?(stride/2):1);
#endif	// _WIN32
			}

		if (bytesRead == 0)
			break;		// key hit
		if (read_p_ret == FALSE)
			break;		// assume acquisition stopped

   		curTime = time(NULL); 		// Get system time

		// every 'printFreq' buffers print status line
		if (nbuffers%printFreq == 0)
			{
			if ((infoBuffer[0] == 0) && (nbuffers > 5))
				{
				// get info delayed because actual stride
				// isn't determined until end of fifth buffer
				GET_DRIVER_INFO(infoBuffer, sizeof(infoBuffer));
				printf("%s\n", infoBuffer);

				// determine stride and sync
				if (stride == 0)
					{
					stride = GET_STRIDE();
					if ((stride < 0) || (stride > 30000))
						stride = 0;
					else if (stride == 0)
//						stride = 1;
//						stride = 131072;
						stride = 64;
					// print every 10th buffer when stride is 64 msec
					printFreq = (stride>640)?1:(640/stride);
					pointerSync = GET_SYNC();
					}
printf("printFreq = %d, stride = %d\n", printFreq, stride);
				}
		
			// determine throughput in words per channel per second	

			int numChannelsXfr = GET_NUM_CHANNELS_XFR();
			int throughPut = GET_CHANNEL_THROUGHPUT();
   			
			dsec = difftime(curTime, startTime);
			if (numChannelsXfr == 0)				
				printf("at second %d, buffer %d, #bytes %d, cycle %lld, seam %d, sample thruput/channel/sec %d\n",
					(int)dsec, nbuffers, (int)bytesRead, cycleNum, (int)seam, throughPut);
			else
				{
				int numReSyncs = GET_NUM_RESYNCS();
				printf("at second %d, buffer %d, #bytes %d, cycle %lld, seam %d, sample thruput/channel/sec %d, #reSyncs %d\n",
					(int)dsec, nbuffers, (int)bytesRead, cycleNum, (int)seam, throughPut, numReSyncs);
				}
			nStatusLines++;		

			if (nStatusLines >= 20)
				{
				printf("****************************************\n");
#ifdef _WIN32
				printf("Hit any key to quit\n");
#else
				printf("Hit ctrl-c to quit\n");
#endif
				printf("****************************************\n");
				nStatusLines = 0;
				}
	
			}

		lastCycleNum = cycleNum;
		lastSeam = seam;

		INT_PTR longs2Check = samBuffer+seam - curBuffer;
		if (longs2Check < 0)
			longs2Check += bytes2Use;
		longs2Check /= 4;

		if (first == TRUE)
			{// determine number of channels
			nch = 0;
			for (int fstSync=0; fstSync<longs2Check; fstSync++)
				if ((unsigned int)((int *)samBuffer)[fstSync] == 0xffffff00)
					{
					if (fstSync != 0)
						printf("1st buffer, 1st sync at byte %d !!!\n", fstSync*4);

					// 2012-05-09
					/* recent hardware changes produce words identical to the sync word
					 * (i.e. 0xffffff00) for channels with no modules installed!
					 * sync detection now requires that some bits in the next word -
					 * the status word - must match those seen in the first status word
					 * of the first acquisition buffer
					 */

					staticStatusBits = ((int *)samBuffer)[fstSync+1] & staticStatusBitsMask; 
					nch = GET_NUM_CHANNELS_USB();
					break;
					}
			if (nch == 0)
				printf("Can't determine number of channels\n");
			else
				printf("number of channels = %d\n", nch);
			first = FALSE;
			}
		
		// check for sync words at the right places and replace them for next pass 		
		int ii;
		int nmissing=0;

		// 2012-05-09: sync test now requires status word be checked also so exit the
		// following loop if there's less than 2 words left to check

		for (ii=0; (ii+1)<longs2Check; ii+=nch,nextSync+=nch)
			{
			if (nextSync*4 >= bytes2Use)
				 nextSync -= (INT)bytes2Use/4;

			int nextStatus = nextSync+1;
			if (nextStatus*4 >= bytes2Use)
				 nextStatus -= (INT)bytes2Use/4;

			if (((unsigned int)((int *)samBuffer)[nextSync] != 0xffffff00) ||
				((((int *)samBuffer)[nextStatus] & staticStatusBitsMask) != staticStatusBits))
				{
				nmissing++;
				printf("missing sync %d: buffer %d, [%d] = %8x [%d] = %8x\n",
					nmissing, nbuffers,
					nextSync, ((int *)samBuffer)[nextSync],
					nextStatus, ((int *)samBuffer)[nextStatus]);
				if (nmissing > 5)
					{
					printf("more than 5 missing syncs in 1 buffer\n");
					break;
					}

				int j;
				for (j=ii; (j+1)<longs2Check; j++,nextSync++)
					{
					if (nextSync*4 >= bytes2Use)
						nextSync -= (INT)bytes2Use/4;

					nextStatus = nextSync+1;
					if (nextStatus*4 >= bytes2Use)
				 	nextStatus -= (INT)bytes2Use/4;

/*					if (((unsigned int)((int *)samBuffer)[nextSync] == 0xffffff00) &&
						((((int *)samBuffer)[nextStatus] & staticStatusBitsMask) ==
							staticStatusBits) &&
						(syncsPresentInRestOfBuffer(samBuffer, nextSync, nch,
								longs2Check) == TRUE))*/
					if (((unsigned int)((int *)samBuffer)[nextSync] == 0xffffff00) &&
						((((int *)samBuffer)[nextStatus] & staticStatusBitsMask) ==
							staticStatusBits))
						{
						printf("sync found again at [%d] = %8x [%d] = %8x\n",
							nextSync, ((int *)samBuffer)[nextSync],
							nextStatus, ((int *)samBuffer)[nextStatus]);
						break;
						}
					}

				if ((j+1) < longs2Check)
					{
					ii = j;
//					((int *)samBuffer)[nextSync] = 0x01234567;
//					lastSync = nextSync;
					continue;
					}
				else
					{
					ii = j;
					printf("no sync found in rest of buffer\n");
					break;
					}
				}

// 20140325  drop write to buffer to allow ring buffer to be tapped out through a tcp socket
//			((int *)samBuffer)[nextSync] = 0x01234567;
//			lastSync = nextSync;
			}

		if (nmissing > 0)
			break;		// missing sync

		// update buffer address
		curBuffer = samBuffer + nextSync*4;
		if (curBuffer >= samBuffer+bytes2Use)
			curBuffer -= bytes2Use;
		
		}  // end of sampling loop

	printf("stopping ...\n");

	/***************************************
	 * stop sample flow into ring buffer
	 * ************************************/
	controlBuffer[0] = 0;

	nStatus = USB_WRITE(usbHandle, (PCHAR)&controlBuffer[0]);
		
	if (nStatus == FALSE)
		{
		// ERROR
		printf ("USB_WRITE for disable handshake trouble %d\n",
			GetLastError()); 
		PrintLastErrorAsText();
		return 7;
		}

	// check for an error code from a remote system,
	// revelant when local samples are coming from a socket
	int remoteErrorCode = GET_REMOTE_ERRORCODE();
	if (remoteErrorCode != 0)
		{
		if (remoteErrorCode == -2)
			{
			puts("*** Transfer buffer overflow on the remote system ***");
			puts("Local socket recv's are not keeping up to remote acquisition rate!");
			puts("Try requesting fewer channels be transferred and/or use a wired connection.");
			}
		else if (remoteErrorCode == -3)
			{
			puts("*** Recv buffer overflow on the this system ***");
			puts("Local READ_POINTER calls are not keeping up to remote acquisition rate!");
			puts("Try requesting fewer channels be transferred and/or use a wired connection.");
			}
		else
			printf("*** Remote sample server sent errorcode %d\n", remoteErrorCode);
		}

	CLOSE_DRIVER(usbHandle);

	// get version of ringbuffer library being used
	infoBuffer[0] = 0;
	GET_DRIVER_INFO(infoBuffer, sizeof(infoBuffer));
	if (infoBuffer[0] != 0)
		printf("%s\n", infoBuffer);

	if (VirtualFree(samBuffer, (SIZE_T)0, MEM_RELEASE) == 0)
		{
		printf("VirtualFree returns 0, GetLastError = %d\n",
			GetLastError());
		PrintLastErrorAsText();
		return 0;
		}

	if ((nsecs > 0) && (sec < nsecs))
		{
#ifdef _WIN32
		printf("Aborted, hit a key to exit\n");
#else
		printf("Aborted, hit ctrl-c to exit\n");
#endif
		for (;;)
			if (_kbhit())
				{
				_getch();
				break;
				}
			else
				Sleep(1000);	// 1 second
		}
	else
		{
#ifdef _WIN32
		printf("Done, hit a key to exit\n");
#else
		printf("Done, hit ctrl-c to exit\n");
#endif
		for (;;)
			if (_kbhit())
				{
				_getch();
				break;
				}
			else
				Sleep(1000);	// 1 second
		}
printf("Done!!!\n");
	return 0; 
	}
