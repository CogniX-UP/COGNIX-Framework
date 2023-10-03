#ifndef BIOSEMI_IO_H
#define BIOSEMI_IO_H
#include <vector>
#include <string>
#include <functional>
#include <memory>

#ifdef _WIN32
#define BIOSEMI_LINKAGE __cdecl
#else
#define BIOSEMI_LINKAGE
#endif


class BiosemiEEG {
public:
	enum Status {Idle, Initializing, Acquiring };
	enum LogType {Normal, Error, Warning };
	// raw sample from the BioSemi amp
	typedef std::vector<int32_t> Sample_T;
	// chunk of raw samples
	typedef std::vector<Sample_T> Chunk;

	// initialize amplifier connection
	BiosemiEEG();
	// shut down amplifier connection
	~BiosemiEEG();

	void ConnectAmplifier();
	void DisconnectAmplifier();
	// get a chunk of raw data
	void GetChunk(Chunk& result);

	// get channel names
	const std::vector<std::string>& ChannelLabels() const { return channelLabels; }
	// get channel types (Sync, Trigger, EEG, EXG,
	const std::vector<std::string>& ChannelTypes() const { return channelTypes; }

	// query amplifier parameters
	bool IsMk2() const { return is_mk2_; }
	bool HasLowBattery() const { return battery_low_; }
	int SpeedMode() const { return speedMode; }
	int SampleRate() const { return sRate; }
	int AllChannelCount() const { return allChanCount; }
	int SyncChannelCount() const { return syncChanCount; }
	int TriggerChannelCount() const { return trigChanCount; }
	int EegChannelCount() const { return eegChanCount; }
	int ExgChannelCount() const { return exgChanCount; }
	int AuxChannelCount() const { return auxChanCount; }
	int AibChannelCount() const { return aibChanCount; }
	void SetLogCallback(std::function<void(const std::string&, LogType logType)>);
	Status GetStatus() { return status; }
private:
	// function handle types for the library IO
	typedef void* (BIOSEMI_LINKAGE* OPEN_DRIVER_ASYNC_t)(void);
	typedef int (BIOSEMI_LINKAGE* USB_WRITE_t)(void*, const unsigned char*);
	typedef int (BIOSEMI_LINKAGE* READ_MULTIPLE_SWEEPS_t)(void*, char*, int);
	typedef int (BIOSEMI_LINKAGE* READ_POINTER_t)(void*, unsigned*);
	typedef int (BIOSEMI_LINKAGE* CLOSE_DRIVER_ASYNC_t)(void*);

	//log callback
	std::function<void(std::string, LogType logType)> logCallback = nullptr;

	Status status = Status::Idle;

	//These could be templated
	void InvokeLog(const std::string &txt, LogType logType = LogType::Normal);
	void InvokeLogError(const std::string &txt);
	// amplifier parameters
	bool is_mk2_;       // whether the amp is a MK2 amplifier
	int speedMode;    // amplifier speed mode
	int sRate;         // native sampling rate
	int syncChanCount;        // number of synchronization channels
	int trigChanCount;        // number of trigger channels
	int eegChanCount;         // number of EEG channels
	int exgChanCount;         // number of ExG channels
	int auxChanCount;			// number of AUX channels
	int aibChanCount;			// number of AIB channels
	int allChanCount;        // total number of channels
	bool battery_low_;  // whether the battery is low

	// ring buffer pointer (from the driver)
	std::unique_ptr<uint32_t> ringBuffer;
	// vector of channel labels (in BioSemi naming scheme)
	std::vector<std::string> channelLabels;
	// vector of channel types (in LSL Semi naming scheme)
	std::vector<std::string> channelTypes;

	int last_idx_;
	// DLL handle
	void* hDLL_;
	// connection handle
	void* hConn_;
	// function pointers
	OPEN_DRIVER_ASYNC_t OPEN_DRIVER_ASYNC;
	USB_WRITE_t USB_WRITE;
	READ_MULTIPLE_SWEEPS_t READ_MULTIPLE_SWEEPS;
	READ_POINTER_t READ_POINTER;
	CLOSE_DRIVER_ASYNC_t CLOSE_DRIVER_ASYNC;
};

#endif // BIOSEMI_IO_H

