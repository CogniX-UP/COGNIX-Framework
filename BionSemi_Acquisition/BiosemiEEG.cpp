#include "BiosemiEEG.h"

#include <iostream>
#include <exception>
#include <vector>
#include <time.h>
#include <sstream>
// DLL location, depending on platform
#ifdef _WIN32
#ifdef _WIN64
const char* dllpath = "DLL/Win64/RingBuffer.dll";
#else
const char* dllpath = "DLL/Win32/RingBuffer.dll";
#endif
#else
#ifdef __macosx__
const char* dllpath = "DLL/Mac/liblabview_dll.0.0.1.dylib";
#else
#ifdef _LP64
const char* dllpath = "DLL/Linux64/liblabview_dll.so";
#else
const char* dllpath = "DLL/Linux32/liblabview_dll.so";
#endif
#endif
#endif

// maximum waiting time when trying to connect
const float max_waiting_time = 3.0;
// size of the initial probing buffer
const int buffer_bytes = 8 * 1024 * 1024;
// preferred size of final buffer (note: must be a multiple of 512; also, ca. 32MB is a good size according to BioSemi)
const int buffer_samples = 60 * 512;

// 64 zero bytes
const unsigned char msg_enable[64] = { 0 };
// 0xFF followed by 63 zero bytes
const unsigned char msg_handshake[64] = { 255,0 };

// library handling
#ifdef _WIN32
#include <windows.h>
#define LOAD_LIBRARY(lpath) LoadLibraryA(lpath)
#define LOAD_FUNCTION(lhandle,fname) GetProcAddress((HMODULE)lhandle,fname)
#define FREE_LIBRARY(lhandle) FreeLibrary((HMODULE)lhandle)
#else
#include <dlfcn.h>
#include "BiosemiEEG.h"
#define LOAD_LIBRARY(lpath) dlopen(lpath,RTLD_NOW | RTLD_LOCAL)
#define LOAD_FUNCTION(lhandle,fname) dlsym(lhandle,fname)
#define FREE_LIBRARY(lhandle) dlclose(lhandle)
#endif

#define tos(x) std::to_string(x)

BiosemiEEG::BiosemiEEG() {
	// === load the library & obtain DLL functions ===
	InvokeLog("Loading BioSemi driver dll...", LogType::Normal);
	hDLL_ = LOAD_LIBRARY(dllpath);
	if (!hDLL_)
		throw std::runtime_error("Could not load BioSemi driver DLL.");
	OPEN_DRIVER_ASYNC = (OPEN_DRIVER_ASYNC_t)LOAD_FUNCTION(hDLL_, "OPEN_DRIVER_ASYNC");
	if (!OPEN_DRIVER_ASYNC) {
		FREE_LIBRARY(hDLL_);
		throw std::runtime_error("Did not find function OPEN_DRIVER_ASYNC() in the BisoSemi driver DLL.");
	}
	USB_WRITE = (USB_WRITE_t)LOAD_FUNCTION(hDLL_, "USB_WRITE");
	if (!USB_WRITE) {
		FREE_LIBRARY(hDLL_);
		throw std::runtime_error("Did not find function USB_WRITE() in the BisoSemi driver DLL.");
	}
	READ_MULTIPLE_SWEEPS = (READ_MULTIPLE_SWEEPS_t)LOAD_FUNCTION(hDLL_, "READ_MULTIPLE_SWEEPS");
	if (!READ_MULTIPLE_SWEEPS) {
		FREE_LIBRARY(hDLL_);
		throw std::runtime_error("Did not find function READ_MULTIPLE_SWEEPS() in the BisoSemi driver DLL.");
	}
	READ_POINTER = (READ_POINTER_t)LOAD_FUNCTION(hDLL_, "READ_POINTER");
	if (!READ_POINTER) {
		FREE_LIBRARY(hDLL_);
		throw std::runtime_error("Did not find function READ_POINTER() in the BisoSemi driver DLL.");
	}
	CLOSE_DRIVER_ASYNC = (CLOSE_DRIVER_ASYNC_t)LOAD_FUNCTION(hDLL_, "CLOSE_DRIVER_ASYNC");
	if (!CLOSE_DRIVER_ASYNC) {
		FREE_LIBRARY(hDLL_);
		throw std::runtime_error("Did not find function CLOSE_DRIVER_ASYNC() in the BisoSemi driver DLL.");
	}
}

BiosemiEEG::~BiosemiEEG() {
	DisconnectAmplifier();
	// close the DLL
	FREE_LIBRARY(hDLL_);
}

void BiosemiEEG::ConnectAmplifier() {
	uint32_t start_idx; // index of the first sample that was recorded
	uint32_t cur_idx;   // index past the current sample

	// === initialize driver ===
	status = Initializing;
	// connect to driver
	InvokeLog("Connecting to driver...");
	hConn_ = OPEN_DRIVER_ASYNC();
	if (!hConn_) {
		status = Idle;
		throw std::runtime_error("Could not open connection to BioSemi driver.");
	}

	// initialize USB2 interface
	InvokeLog("Initializing USB interface...");
	if (!USB_WRITE(hConn_, &msg_enable[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Could not initialize BioSemi USB2 interface.");
	}


	// === allocate ring buffer and begin acquisiton ===

	// initialize the initial (probing) ring buffer
	InvokeLog("Initializing ring buffer...");
	
	ringBuffer.reset(new uint32_t[buffer_bytes]);
	
	if (!ringBuffer) {
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Could not allocate ring buffer (out of memory).");
	}
	memset(ringBuffer.get(), 0, buffer_bytes);

	// begin acquisition
	READ_MULTIPLE_SWEEPS(hConn_, (char*)ringBuffer.get(), buffer_bytes);

	// enable handshake
	InvokeLog("Enabling handshake...");
	if (!USB_WRITE(hConn_, &msg_handshake[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Could not enable handshake with BioSemi USB2 interface.");
	}


	// === read the first sample ===

	// obtain buffer head pointer
	InvokeLog("Querying buffer pointer...");
	if (!READ_POINTER(hConn_, &start_idx)) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Can not obtain ring buffer pointer from BioSemi driver.");
	}

	// check head pointer validity
	if (start_idx > buffer_bytes) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Buffer pointer returned by BioSemi driver is not in the valid range.");
	}

	// read the first sample
	InvokeLog("Waiting for data...");
	clock_t start_time = clock();
	while (1) {
		// error checks...
		if (!READ_POINTER(hConn_, &cur_idx)) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			status = Idle;
			throw std::runtime_error("Ring buffer handshake with BioSemi driver failed unexpectedly.");
		}
		if (((double)(clock() - start_time)) / CLOCKS_PER_SEC > max_waiting_time) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			if (cur_idx - start_idx < 8) {
				status = Idle;
				throw std::runtime_error("BioSemi driver does not transmit data. Is the box turned on?");
			}
			else {
				status = Idle;
				throw std::runtime_error("Did not get a sync signal from BioSemi driver. Is the battery charged?");
			}
		}

		if ((cur_idx - start_idx >= 8) && (ringBuffer.get()[0] == 0xFFFFFF00)) {
			// got a sync signal on the first index...
			start_idx = 0;
			break;
		}
		if ((cur_idx - start_idx >= 8) && (ringBuffer.get()[start_idx / 4] == 0xFFFFFF00))
			// got the sync signal!
			break;
	}


	// === parse amplifier configuration ===

	// read the trigger channel data ...
	InvokeLog("Checking status...");
	uint32_t deviceStatus = ringBuffer.get()[start_idx / 4 + 1] >> 8;
	InvokeLog("Status: " + tos(deviceStatus));

	// determine channel configuration
	is_mk2_ = ((deviceStatus & (1 << 23)) != 0);
	InvokeLog("MK2: " + tos(is_mk2_));

	// check speed mode
	speedMode = ((deviceStatus & (1 << 17)) >> 17) + ((deviceStatus & (1 << 18)) >> 17) + ((deviceStatus & (1 << 19)) >> 17) + ((deviceStatus & (1 << 21)) >> 18);
	InvokeLog("Speed Mode: " + tos(speedMode));

	// check for problems...
	if (speedMode > 9) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		if (is_mk2_) {
			throw std::runtime_error("BioSemi amplifier speed mode wheel must be between positions 0 and 8 (9 is a reserved value); recommended for typical use is 4.");
			status = Idle;
		}	
		else {
			throw std::runtime_error("BioSemi amplifier speed mode wheel must be between positions 0 and 8 (9 is a reserved value); recommended for typical use is 4.");
			status = Idle;
		}
	}

	// determine sampling rate (http://www.biosemi.com/faq/adjust_samplerate.htm)
	switch (speedMode & 3) {
	case 0: sRate = 2048; break;
	case 1: sRate = 4096; break;
	case 2: sRate = 8192; break;
	case 3: sRate = 16384; break;
	}
	// speed modes lower than 3 are special on Mk2 and are for daisy-chained setups (@2KHz)
	bool multibox = false;
	if (is_mk2_ && speedMode < 4) {
		sRate = 2048;
		multibox = true;
	}

	InvokeLog("Sampling Rate: " + tos(sRate));

	// determine channel configuration -- this is written according to:
	//   http://www.biosemi.com/faq/make_own_acquisition_software.htm
	//   http://www.biosemi.com/faq/adjust_samplerate.htm
	if (is_mk2_) {
		// in an Mk2 the speed modes 0-3 are for up to 4 daisy-chained boxes; these are 
		// multiplexed, have 128ch EEG each and 8ch EXG each, plus 16 extra channels each (howdy!)
		switch (speedMode) {
		case 0:
		case 1:
		case 2:
		case 3:
			eegChanCount = 4 * 128; exgChanCount = 4 * 8; auxChanCount = 4 * 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 610; break;
			// spd modes 4-7 are the regular ones and have 8 EXG's added in
		case 4: eegChanCount = 256; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 282; break;
		case 5: eegChanCount = 128; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 154; break;
		case 6: eegChanCount = 64; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 90; break;
		case 7: eegChanCount = 32; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 58; break;
			// spd mode 8 adds
		case 8: eegChanCount = 256; exgChanCount = 8; auxChanCount = 16; aibChanCount = 32; trigChanCount = 1; syncChanCount = 1; allChanCount = 314; break;
		}
	}
	else {
		// in a Mk1 the EXG's are off in spd mode 0-3 and on in spd mode 4-7 (but subtracted from the EEG channels)
		switch (speedMode) {
			// these are all-EEG modes
		case 0: eegChanCount = 256; exgChanCount = 0; auxChanCount = 0; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 258; break;
		case 1: eegChanCount = 128; exgChanCount = 0; auxChanCount = 0; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 130; break;
		case 2: eegChanCount = 64; exgChanCount = 0; auxChanCount = 0; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 66; break;
		case 3: eegChanCount = 32; exgChanCount = 0; auxChanCount = 0; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 34; break;
			// in these modes there are are 8 EXGs and 16 aux channels
		case 4: eegChanCount = 232; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 258; break;
		case 5: eegChanCount = 104; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 130; break;
		case 6: eegChanCount = 40; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 66; break;
		case 7: eegChanCount = 8; exgChanCount = 8; auxChanCount = 16; aibChanCount = 0; trigChanCount = 1; syncChanCount = 1; allChanCount = 34; break;
			// in spd mode 8 there are 32 AIB channels from an Analog Input Box (AIB) multiplexed in (EXGs are off)
		case 8: eegChanCount = 256; exgChanCount = 0; auxChanCount = 0; aibChanCount = 32; trigChanCount = 1; syncChanCount = 1; allChanCount = 290; break;
		}
	}

	if (logCallback) {
		std::ostringstream oss;
		oss << "Channels: " << allChanCount << "(" << syncChanCount << "xSync, " << trigChanCount << "xTrigger, " << eegChanCount << "xEEG, " << exgChanCount << "xExG, " << auxChanCount << "xAUX, " << aibChanCount << "xAIB)";
		InvokeLog(oss.str());
	}

	// check for additional problematic conditions
	battery_low_ = (deviceStatus & (1 << 22)) != 0;
	if (battery_low_) {
		InvokeLog("Battery: Low");
		InvokeLog("Amplifier will shut down within 30-60 minutes");
	}
	else
		InvokeLog("Battery: OK!");

	// compute a proper buffer size (needs to be a multiple of 512, a multiple of nbchan, as well as ~32MB in size)
	InvokeLog("Reallocating optimized ring buffer...");

	// === shutdown current coonnection ===

	// shutdown current connection
	InvokeLog("Sending the enable message again...");
	if (!USB_WRITE(hConn_, &msg_enable[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Error while disabling the handshake.");
	}

	InvokeLog("Closing the driver...");
	if (!CLOSE_DRIVER_ASYNC(hConn_)) {
		status = Idle;
		throw std::runtime_error("Error while disconnecting.");
	}

	// === reinitialize acquisition ===
	InvokeLog("Allocating a new ring buffer...");
	ringBuffer.reset(new uint32_t[buffer_samples * allChanCount]);

	if (!ringBuffer) {
		status = Idle;
		throw std::runtime_error("Could not reallocate ring buffer (out of memory?).");
	}

	InvokeLog("Zeroing the ring buffer...");

	memset(ringBuffer.get(), 0, buffer_samples * 4 * allChanCount);

	// reconnect to driver

	InvokeLog("Opening the driver...");

	hConn_ = OPEN_DRIVER_ASYNC();
	if (!hConn_) {
		status = Idle;
		throw std::runtime_error("Could not open connection to BioSemi driver.");
	}
	// reinitialize USB2 interface

	InvokeLog("Reinitializing the USB interface...");
	if (!USB_WRITE(hConn_, &msg_enable[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Could not initialize BioSemi USB2 interface.");
	}
	// begin acquisition

	InvokeLog("Starting data acquisition...");

	READ_MULTIPLE_SWEEPS(hConn_, (char*)ringBuffer.get(), buffer_samples * 4 * allChanCount);
	// enable handshake
	InvokeLog("Enabling handshake...");

	if (!USB_WRITE(hConn_, &msg_handshake[0])) {
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Could not reenable handshake with BioSemi USB2 interface.");
	}


	// === check for correctness of new data ===

	// make sure that we can read the buffer pointer
	InvokeLog("Attempt to read buffer pointer...");

	if (!READ_POINTER(hConn_, &start_idx)) {
		USB_WRITE(hConn_, &msg_enable[0]);
		CLOSE_DRIVER_ASYNC(hConn_);
		status = Idle;
		throw std::runtime_error("Can not obtain ring buffer pointer from BioSemi driver.");
	}

	InvokeLog("Verifying channel format...");

	start_time = clock();
	while (1) {
		// error checks
		if (!READ_POINTER(hConn_, &cur_idx)) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			status = Idle;
			throw std::runtime_error("Ring buffer handshake with BioSemi driver failed unexpectedly.");
		}
		if (((double)(clock() - start_time)) / CLOCKS_PER_SEC > max_waiting_time) {
			USB_WRITE(hConn_, &msg_enable[0]);
			CLOSE_DRIVER_ASYNC(hConn_);
			if (cur_idx - start_idx < 8) {
				status = Idle;
				throw std::runtime_error("BioSemi driver does not transmit data. Is the box turned on?");
			}
			else 
			{
				status = Idle;
				throw std::runtime_error("Did not get a sync signal from BioSemi driver. Is the battery charged?");	
			}
		}
		if ((cur_idx - start_idx >= 4 * allChanCount) && (ringBuffer.get()[0] == 0xFFFFFF00))
			// got a sync signal on the first index
			start_idx = 0;
		if ((cur_idx - start_idx >= 4 * allChanCount) && (ringBuffer.get()[start_idx / 4] == 0xFFFFFF00)) {
			if (ringBuffer.get()[start_idx / 4 + allChanCount] != 0xFFFFFF00) {
				USB_WRITE(hConn_, &msg_enable[0]);
				CLOSE_DRIVER_ASYNC(hConn_);
				status = Idle;
				throw std::runtime_error("Sync signal did not show up at the expected position.");
			}
			else {
				InvokeLog("Channel format is correct...");
				// all correct
				break;
			}
		}
	}


	// === derive channel labels ===

	channelLabels.clear();
	channelTypes.clear();
	for (int k = 1; k <= syncChanCount; k++) {
		channelLabels.push_back(std::string("Sync") += std::to_string(k));
		channelTypes.push_back(syncType);
	}
	for (int k = 1; k <= trigChanCount; k++) {
		channelLabels.push_back(std::string("Trig") += std::to_string(k));
		channelTypes.push_back(triggerType);
	}

	//Won't look at this for now - it's for speed less than 4
	if (multibox) {
		// multi-box setup
		for (int b = 0; b <= 3; b++) {
			const char* boxid[] = { "_Box1","_Box2","_Box3","_Box4" };
			
			for (int k = 1; k <= eegChanCount / 4; k++) {
				std::string tmp = "A"; 
				
				tmp[0] = 'A' + (k - 1) / 32;
				
				channelLabels.push_back((std::string(tmp) += std::to_string(1 + (k - 1) % 32)) += boxid[b]);
				channelTypes.push_back(eegType);
			}

			for (int k = 1; k <= exgChanCount / 4; k++) {
				channelLabels.push_back((std::string("EX") += std::to_string(k)) += boxid[b]);
				channelTypes.push_back(exgType);
			}

			for (int k = 1; k <= auxChanCount / 4; k++) {
				channelLabels.push_back((std::string("AUX") += std::to_string(k)) += boxid[b]);
				channelTypes.push_back(auxType);
			}

			for (int k = 1; k <= aibChanCount / 4; k++) {
				channelLabels.push_back((std::string("AIB") += std::to_string(k)) += boxid[b]);
				channelTypes.push_back(aibType);
			}
		}
	}
	else {
		// regular setup
		for (int k = 1; k <= eegChanCount; k++) {
			//BioSemi might have A-H, hence we use index k to create the A1-H32
			std::string tmp = "A"; 
			tmp[0] = 'A' + (k - 1) / 32;
			channelLabels.push_back(std::string(tmp) += std::to_string(1 + (k - 1) % 32));
			channelTypes.push_back(eegType);
		}

		for (int k = 1; k <= exgChanCount; k++) {
			channelLabels.push_back(std::string("EX") += std::to_string(k));
			channelTypes.push_back(exgType);
		}

		for (int k = 1; k <= auxChanCount; k++) {
			channelLabels.push_back(std::string("AUX") += std::to_string(k));
			channelTypes.push_back(auxType);
		}

		for (int k = 1; k <= aibChanCount; k++) {
			channelLabels.push_back(std::string("AIB") += std::to_string(k));
			channelTypes.push_back(aibType);
		}
	}

	last_idx_ = 0;

	status = Acquiring;
	InvokeLog("Acquiring!!");
}

void BiosemiEEG::DisconnectAmplifier() {
	status = Idle;
	auto pointer = ringBuffer.release();
	if (pointer)
		delete pointer;

	/* close driver connection */
	if (!USB_WRITE(hConn_, &msg_enable[0]))
		InvokeLog("The handshake while shutting down the BioSemi driver gave an error.");
	if (!CLOSE_DRIVER_ASYNC(hConn_))
		InvokeLog("Closing the BioSemi driver gave an error.");
}

void BiosemiEEG::GetChunk(Chunk& result) {
	// get current buffer offset
	int cur_idx;
	if (!READ_POINTER(hConn_, (unsigned*)&cur_idx))
		throw std::runtime_error("Reading the updated buffer pointer gave an error.");
	cur_idx = cur_idx / 4;

	// forget about incomplete sample data
	cur_idx = cur_idx - cur_idx % allChanCount;
	if (cur_idx < 0)
		cur_idx = cur_idx + buffer_samples * allChanCount;

	result.clear();
	if (cur_idx != last_idx_) {
		if (cur_idx > last_idx_) {
			// sequential read: copy intermediate part between offsets
			int chunklen = (cur_idx - last_idx_) / allChanCount;
			result.resize(chunklen);
			for (int k = 0; k < chunklen; k++) {
				result[k].resize(allChanCount);
				memcpy(&result[k][0], &ringBuffer.get()[last_idx_ + k * allChanCount], allChanCount * 4);
			}
		}
		else {
			// wrap-around read: concatenate two parts
			int chunklen = (cur_idx + buffer_samples * allChanCount - last_idx_) / allChanCount;
			result.resize(chunklen);
			int first_section = buffer_samples - last_idx_ / allChanCount;
			for (int k = 0; k < first_section; k++) {
				result[k].resize(allChanCount);
				memcpy(&result[k][0], &ringBuffer.get()[last_idx_ + k * allChanCount], allChanCount * 4);
			}
			int second_section = chunklen - first_section;
			for (int k = 0; k < second_section; k++) {
				result[first_section + k].resize(allChanCount);
				memcpy(&result[first_section + k][0], &ringBuffer.get()[k * allChanCount], allChanCount * 4);
			}
		}
		// update status flags
		uint32_t deviceStatus = ringBuffer.get()[cur_idx] >> 8;
		battery_low_ = (deviceStatus & (1 << 22)) != 0;
	}

	// update last buffer pointer
	last_idx_ = cur_idx;
}

void BiosemiEEG::SetLogCallback(std::function<void(const std::string&, LogType logType)> callback) {
	logCallback = callback;
}

void BiosemiEEG::InvokeLog(const std::string &txt, LogType logType) {
	if (logCallback)
		logCallback(txt, logType);
}

void BiosemiEEG::InvokeLogError(const std::string& txt) {
	if (logCallback)
		logCallback(txt, LogType::Error);
}