/*
 * Public ringbuffer header file
 *
 * Copyright (C) 2003-2015 Biosemi <www.biosemi.com>
 */

#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LABVIEW_DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LABVIEW_DLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifdef _WIN32
#ifdef LABVIEW_DLL_EXPORTS
#define LABVIEW_DLL_API __declspec(dllexport) 
#else
#define LABVIEW_DLL_API __declspec(dllimport) 
#endif // LABVIEW_DLL_EXPORTS
#endif // _WIN32

#ifdef _WIN32
#ifdef __cplusplus
// This class is exported from the Ringbuffer.dll
class LABVIEW_DLL_API CLabview_DLL {
public:
	CLabview_DLL(void);
	// TODO: add your methods here.
};
#endif	// __cplusplus
#else
// linux
#define LABVIEW_DLL_API
#endif

#ifdef _WIN32
#define LINKAGE __cdecl
#else
// linux
#define LINKAGE
#endif	// _WIN32

#ifndef __cplusplus
#define BOOL	int
#define FALSE	0
#define TRUE	1
#endif	// __cplusplus

#ifndef _WIN32
// linux
#include <stdint.h>
#include <stddef.h>
#define HANDLE	void*
#ifdef __cplusplus
#ifndef BOOL
#define	BOOL	bool
#endif	// BOOL
#ifndef FALSE
#define FALSE	false
#endif	// FALSE
#ifndef TRUE
#define TRUE	true
#endif	// TRUE
#endif	// __cplusplus
#define CHAR	char
#define PCHAR	char*
#define PUCHAR	unsigned char*
#define DWORD	int
#define SIZE_T	size_t
#define ULONG	unsigned long
#define PULONG	unsigned long*
#define INT_PTR	intptr_t
#define PINT_PTR	intptr_t*
#define INT	int
#define VOID	void
#define FLOAT	float
#endif

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus
//LABVIEW_DLL_API LINKAGE HANDLE OPEN_DRIVER_ASYNC(void) __attribute__((cdecl));
LABVIEW_DLL_API HANDLE LINKAGE OPEN_DRIVER_ASYNC(void);
LABVIEW_DLL_API HANDLE LINKAGE OPEN_DRIVER(void);
LABVIEW_DLL_API BOOL LINKAGE READ_MULTIPLE_SWEEPS(HANDLE hdevice,PCHAR data,INT_PTR nBytesToRead); 
LABVIEW_DLL_API BOOL LINKAGE WRITE_SINGLE_SWEEP(HANDLE hdevice,PCHAR data,DWORD nBytesToWrite);
LABVIEW_DLL_API BOOL LINKAGE WRITE_SINGLE_SWEEP_ASYNC(HANDLE hdevice,PCHAR data,DWORD nBytesToWrite);
LABVIEW_DLL_API BOOL LINKAGE CLOSE_DRIVER_ASYNC(HANDLE hdevice);
LABVIEW_DLL_API BOOL LINKAGE CLOSE_DRIVER(HANDLE hdevice);
LABVIEW_DLL_API BOOL LINKAGE READ_POINTER(HANDLE hdevice,PINT_PTR pointer);
LABVIEW_DLL_API BOOL LINKAGE READ_POINTER_WITH_CYCLE(HANDLE hdevice,long long *cycleNum,PINT_PTR pointer);
LABVIEW_DLL_API BOOL LINKAGE USB_WRITE(HANDLE hdevice, PCHAR data);
LABVIEW_DLL_API FLOAT LINKAGE GET_BYTES_PER_MSEC(void);
LABVIEW_DLL_API VOID LINKAGE SET_SYNC(BOOL onOff);
LABVIEW_DLL_API BOOL LINKAGE GET_SYNC(void);
LABVIEW_DLL_API INT LINKAGE GET_STRIDE(void);
LABVIEW_DLL_API VOID LINKAGE SET_STRIDE_MS(int msec);
LABVIEW_DLL_API VOID LINKAGE SET_STRIDE_KB(int kBytes);
LABVIEW_DLL_API BOOL LINKAGE SET_TRANSFER_CHANNEL_REQUEST_RANGES(int numRanges, int *channelRanges);
LABVIEW_DLL_API INT LINKAGE GET_NUM_CHANNELS_XFR(void);
LABVIEW_DLL_API INT LINKAGE GET_NUM_CHANNELS_USB(void);
LABVIEW_DLL_API INT LINKAGE GET_NUM_RESYNCS(void);
LABVIEW_DLL_API INT LINKAGE GET_NUM_RESYNCS(void);
LABVIEW_DLL_API INT LINKAGE GET_REMOTE_ERRORCODE(void);
LABVIEW_DLL_API INT LINKAGE GET_CHANNEL_THROUGHPUT(void);
LABVIEW_DLL_API INT LINKAGE GET_REMOTE_CHANNEL_THROUGHPUT(void);
LABVIEW_DLL_API BOOL LINKAGE DOWNLOAD_FX2(void);
LABVIEW_DLL_API BOOL LINKAGE DOWNLOAD_FX2_EEPROM(void); 
LABVIEW_DLL_API BOOL LINKAGE RESET_FX2(BOOL RESET);
LABVIEW_DLL_API BOOL LINKAGE DOWNLOAD_FX2_IIC_FIRMWARE(void);
LABVIEW_DLL_API PCHAR LINKAGE GET_DRIVER_INFO(PCHAR infoBuffer, SIZE_T infoSize);
LABVIEW_DLL_API PCHAR LINKAGE GET_HARDWARE_PARAMETER(INT which, PCHAR infoBuffer, SIZE_T infoSize);
LABVIEW_DLL_API VOID LINKAGE SET_LISTEN_PORT(const char *port);
LABVIEW_DLL_API VOID LINKAGE SET_CONNECT_NODE_PORT(const char *node, const char *port);
#ifdef __cplusplus
}
#endif	// __cplusplus

#endif	//  __RINGBUFFER_H__
