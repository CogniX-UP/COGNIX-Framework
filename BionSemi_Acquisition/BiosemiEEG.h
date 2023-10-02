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
	// raw sample from the BioSemi amp
	typedef std::vector<int32_t> Sample_T;
	// chunk of raw samples
	typedef std::vector<Sample_T> Chunk;

	// initialize amplifier connection
	BiosemiEEG();
	// shut down amplifier connection
	~BiosemiEEG();

	void ConnectAmplifier();
	// get a chunk of raw data
	void GetChunk(Chunk& result);

	// get channel names
	const std::vector<std::string>& channel_labels() const { return channelLabels; }
	// get channel types (Sync, Trigger, EEG, EXG,
	const std::vector<std::string>& channel_types() const { return channelTypes; }

	// query amplifier parameters
	bool IsMk2() const { return is_mk2_; }
	bool HasLowBattery() const { return battery_low_; }
	int SpeedMode() const { return speed_mode_; }
	int srate() const { return srate_; }
	int AllChannelCount() const { return nbchan_; }
	int SyncChannelCount() const { return nbsync_; }
	int TriggerChannelCount() const { return nbtrig_; }
	int EegChannelCount() const { return nbeeg_; }
	int ExgChannelCount() const { return nbexg_; }
	int AuxChannelCount() const { return nbaux_; }
	int AibChannelCount() const { return nbaib_; }
	void SetLogCallback(std::function<void(const std::string&)>);

private:
	// function handle types for the library IO
	typedef void* (BIOSEMI_LINKAGE* OPEN_DRIVER_ASYNC_t)(void);
	typedef int (BIOSEMI_LINKAGE* USB_WRITE_t)(void*, const unsigned char*);
	typedef int (BIOSEMI_LINKAGE* READ_MULTIPLE_SWEEPS_t)(void*, char*, int);
	typedef int (BIOSEMI_LINKAGE* READ_POINTER_t)(void*, unsigned*);
	typedef int (BIOSEMI_LINKAGE* CLOSE_DRIVER_ASYNC_t)(void*);

	//log callback
	std::function<void(std::string)> logCallback = nullptr;
	void InvokeLog(const std::string &txt);
	// amplifier parameters
	bool is_mk2_;       // whether the amp is a MK2 amplifier
	int speed_mode_;    // amplifier speed mode
	int srate_;         // native sampling rate
	int nbsync_;        // number of synchronization channels
	int nbtrig_;        // number of trigger channels
	int nbeeg_;         // number of EEG channels
	int nbexg_;         // number of ExG channels
	int nbaux_;			// number of AUX channels
	int nbaib_;			// number of AIB channels
	int nbchan_;        // total number of channels
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

