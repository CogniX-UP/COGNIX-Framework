/*
 * Public bsif header file
 *
 * Copyright (C) 2003-2015 Biosemi <www.biosemi.com>
 */

#ifndef __BSIF_H__
#define __BSIF_H__

#ifdef _WIN32
#define LINKAGE __cdecl
#else
// linux
#define LINKAGE
#endif

#ifndef __cplusplus
#define BOOL    int
#define FALSE   0
#define TRUE    1
#endif  // __cplusplus

#ifndef _WIN32
// linux
#include <stdint.h>
#include <stddef.h>
#define HANDLE  void*
#ifdef __cplusplus
#ifndef BOOL
#define BOOL    bool
#endif	// BOOL
#ifndef FALSE
#define FALSE   false
#endif	// FALSE
#ifndef TRUE
#define TRUE    true
#endif	// TRUE
#endif	// __cplusplus
#define CHAR    char
#define PCHAR   char*
#define PUCHAR  unsigned char*
#define DWORD   int
#define SIZE_T  size_t
#define ULONG   unsigned long
#define PULONG  unsigned long*
#define INT_PTR intptr_t
#define PINT_PTR        intptr_t*
#define INT     int
#define VOID	void
#define LPVOID	void*
#define FLOAT	float
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
BOOL LINKAGE BSIF_READ_POINTER(HANDLE hdevice, PINT_PTR pointer);
BOOL LINKAGE BSIF_READ_POINTER_WITH_CYCLE(HANDLE hdevice, long long *cycleNum, PINT_PTR pointer);
BOOL LINKAGE BSIF_READ_POINTER_SYNCED(HANDLE hdevice, long long *cycleNum, PINT_PTR pointer);
VOID LINKAGE BSIF_TAP_READ_POINTER(HANDLE hdevice, PINT_PTR pointer);
BOOL LINKAGE BSIF_INJECT_READ_POINTER(HANDLE hdevice, PINT_PTR pointer);
BOOL LINKAGE BSIF_INJECT_READ_POINTER_WITH_CYCLE(HANDLE hdevice, long long *cycleNum, PINT_PTR pointer);
VOID LINKAGE BSIF_SET_DEBUG(BOOL onoff);
VOID LINKAGE BSIF_TAP_SET_DEBUG(BOOL onoff, const char *);
VOID LINKAGE BSIF_INJECT_SET_DEBUG(BOOL onoff, const char *);
VOID LINKAGE BSIF_SET_LOG(BOOL onoff);
VOID LINKAGE BSIF_TAP_SET_LOG(BOOL onoff, const char *);
VOID LINKAGE BSIF_INJECT_SET_LOG(BOOL onoff, const char *);
VOID LINKAGE BSIF_SET_SYNC(BOOL onoff);
VOID LINKAGE BSIF_INJECT_SET_SYNC(BOOL onoff);
BOOL LINKAGE BSIF_GET_SYNC();
BOOL LINKAGE BSIF_INJECT_GET_SYNC();
VOID LINKAGE BSIF_SET_STRIDE(int msec);
VOID LINKAGE BSIF_SET_STRIDE_MS(int msec);
VOID LINKAGE BSIF_SET_STRIDE_KB(int kbytes);
INT LINKAGE BSIF_GET_STRIDE();
INT LINKAGE BSIF_INJECT_GET_STRIDE();
HANDLE LINKAGE BSIF_OPEN_DRIVER();
HANDLE LINKAGE BSIF_TAP_OPEN_DRIVER();
HANDLE LINKAGE BSIF_INJECT_OPEN_DRIVER();
BOOL LINKAGE BSIF_READ_MULTIPLE_SWEEPS(HANDLE WinUsbHandle, PCHAR data, INT_PTR nBytesToRead);
BOOL LINKAGE BSIF_TAP_READ_MULTIPLE_SWEEPS(HANDLE WinUsbHandle, PCHAR data, INT_PTR nBytesToRead);
BOOL LINKAGE BSIF_INJECT_READ_MULTIPLE_SWEEPS(HANDLE WinUsbHandle, PCHAR data, INT_PTR nBytesToRead);
BOOL LINKAGE BSIF_USB_WRITE(HANDLE hdevice, PCHAR data);
BOOL LINKAGE BSIF_INJECT_USB_WRITE(HANDLE hdevice, PCHAR data);
FLOAT LINKAGE BSIF_GET_BYTES_PER_MSEC();
FLOAT LINKAGE BSIF_INJECT_GET_BYTES_PER_MSEC();
BOOL LINKAGE BSIF_CLOSE_DRIVER(HANDLE hdevice);
BOOL LINKAGE BSIF_TAP_CLOSE_DRIVER(HANDLE hdevice);
BOOL LINKAGE BSIF_INJECT_CLOSE_DRIVER(HANDLE hdevice);
PCHAR LINKAGE BSIF_GET_DRIVER_INFO(PCHAR infoBuffer, SIZE_T infoSize);
PCHAR LINKAGE BSIF_GET_HARDWARE_PARAMETER(INT which, PCHAR infoBuffer, SIZE_T infoSize);
PCHAR LINKAGE BSIF_INJECT_GET_DRIVER_INFO(PCHAR infoBuffer, SIZE_T infoSize);
BOOL LINKAGE BSIF_DOWNLOAD_FX2_IIC_FIRMWARE();
BOOL LINKAGE BSIF_DOWNLOAD_FX2_EEPROM();
VOID BSIF_OPEN_KEYBOARD(VOID);
VOID BSIF_CLOSE_KEYBOARD(VOID);
BOOL BSIF_READ_KEYBOARD(int *keycode);
VOID BSIF_MY_LOAD();
VOID BSIF_MY_UNLOAD();

VOID LINKAGE BSIF_TAP_SET_LISTEN_PORT(const char *);
VOID LINKAGE BSIF_INJECT_SET_CONNECT_NODE_PORT(const char *node, const char *port);

INT LINKAGE BSIF_Setup_Server_Socket(const char *, VOID (*f)(BOOL, const char *));
INT LINKAGE BSIF_Wait_For_Connection(int listenfd, int numSecs, char *clientNode, size_t clientNodeDim, BOOL, VOID (*f)(BOOL, const char *));
BOOL LINKAGE BSIF_Socket_Send(int new_fd, PUCHAR buf, size_t size, BOOL, VOID (*f)(BOOL, const char *));
BOOL LINKAGE BSIF_Socket_Send_OOB(int new_fd, PUCHAR buf, size_t size, BOOL, VOID (*f)(BOOL, const char *));
INT LINKAGE BSIF_Socket_Recv(int new_fd, PUCHAR buf, size_t size, int numSecs, BOOL, VOID (*f)(BOOL, const char *));
INT LINKAGE BSIF_Socket_Recv_OOB(int new_fd, PUCHAR buf, size_t size, int numSecs, BOOL, VOID (*f)(BOOL, const char *));
VOID LINKAGE BSIF_Close_Socket(int new_fd, VOID (*f)(BOOL, const char *));
INT LINKAGE BSIF_Connect_To_Sample_Server(const char *node, const char *port, int numSecs, VOID (*f)(BOOL, const char *));
BOOL LINKAGE BSIF_INJECT_SET_TRANSFER_CHANNEL_REQUEST_RANGES(int numRanges, int *channelRanges);
INT LINKAGE BSIF_INJECT_GET_REMOTE_ERRORCODE();
VOID LINKAGE BSIF_INJECT_SET_STRIDE_MS(int msec);
VOID LINKAGE BSIF_INJECT_SET_STRIDE_KB(int kBytes);
INT LINKAGE BSIF_INJECT_GET_NUM_CHANNELS_XFR();
INT LINKAGE BSIF_INJECT_GET_NUM_RESYNCS();
INT LINKAGE BSIF_INJECT_GET_REMOTE_CHANNEL_THROUGHPUT();

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif	// __BSIF_H__
