# use with Visual Studio's "nmake" command
ALL: RingBuffer_SyncTest.exe

# /wd4996 supresses deprecated function warnings
# /Wall requests all warnings be listed
# /DLEGACY uses unsafe string functions, needed for XP
OPTS=/wd4996 /Wall
#OPTS=/wd4996

RingBuffer.lib: RingBuffer.dll

RingBuffer.dll: ringbuffer.obj bsif_winusb.obj \
	bsif_tap_usb_samples.obj bsif_inject_socket_samples.obj \
	bsif_socket_io.obj 
	link /dll $** /out:$@ ws2_32.lib setupapi.lib

ringbuffer_synctest.obj: ../C-code/ringbuffer_synctest.cpp
	cl /c $(OPTS) $**

RingBuffer_SyncTest.exe: ringbuffer_synctest.obj RingBuffer.lib
	link $** /out:$@

ringbuffer.obj: ../C-code/ringbuffer.cpp
	cl /c $(OPTS) $**

clean:
	del ringbuffer.obj
	del ringbuffer_synctest.obj
	del RingBuffer.DLL
	del ringbuffer.lib
	del ringbuffer.exp
	del RingBuffer_SyncTest.exe
