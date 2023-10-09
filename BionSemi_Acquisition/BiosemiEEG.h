/**
 * @file EEG/BiosemiEEG.h
 * @author Georgios Papadoulis
 *
 * BioSemi acquisition. A modified version taken from an old LSL application for BioSemi.
 * Remade to be programmer and extension friendly.
 *
 */

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

/**
* The class BiosemiEEG for connecting with a BioSemi device and sending acquiring raw data.
* It provides safety mechanisms as exceptions in case the acquisition doesn't work. The order
* that the channels are read from is Synch -> Triggers -> EEG -> EXG (EXT) -> AUX -> Analog.
* Though daisy-channels (multiple boxes) are supported, the LSL implementation hasn't taken them into account.
*/
class BiosemiEEG {
public:
	/**
	* Keywords for the various types of channels.
	*/
	inline static const std::string
		syncType = "Sync",
		triggerType = "Trigger",
		eegType = "EEG",
		exgType = "EXG",
		auxType = "AUX",
		aibType = "Analog";

	enum Status {Idle, Initializing, Acquiring };
	enum LogType {Normal, Error, Warning };
	// raw sample from the BioSemi amp
	typedef std::vector<int32_t> Sample_T;
	// chunk of raw samples
	typedef std::vector<Sample_T> Chunk;

	BiosemiEEG();
	~BiosemiEEG();

	/**
	* Initializes a USB connection with the amplifier. If connected, data is continuously read to a buffer.
	*/
	void ConnectAmplifier();
	/**
	* Closes the USB connection to the amplifier.
	*/
	void DisconnectAmplifier();
	/**
	* Fills an external chunk vector [num_samples x num_acquisitions] with the current buffer data, which is then reset.
	* @param result The external chunk vector.
	*/
	void GetChunk(Chunk& result);

	/**
	* The channel labels as typically indicated by BioSemi [A1-H32] for EEG. A more complex set-up
	* is given for multi-chained (daisy) boxes, but won't be discussed here.
	* @return Channel Labels for BioSemi
	*/
	const std::vector<std::string>& ChannelLabels() const { return channelLabels; }

	/**
	* The channel types, such as Sync, Trigger, EEG etc.
	* @return Channel Types for BioSemi
	*/
	const std::vector<std::string>& ChannelTypes() const { return channelTypes; }

	/**
	* Whether the box is MK1 or MK2.
	* @return boolean indicating MK2 if true
	*/
	bool IsMk2() const { return is_mk2_; }
	/**
	* If the box has low battery.
	* @return boolean indicating low battery status
	*/
	bool HasLowBattery() const { return battery_low_; }
	/**
	* The speed mode of the device. Typically the sampling rate, check the manual
	* for further information.
	* @return an integer in [1-8] range
	*/
	int SpeedMode() const { return speedMode; }
	/**
	* The sampling rate
	* @return the sampling rate of the device
	*/
	int SampleRate() const { return sRate; }
	/**
	* @return the total channels of this device
	*/
	int AllChannelCount() const { return allChanCount; }
	/**
	* @return the number of sync channels (1 per box)
	*/
	int SyncChannelCount() const { return syncChanCount; }
	/**
	* @return the number of trigger channels (1 per box)
	*/
	int TriggerChannelCount() const { return trigChanCount; }
	/**
	* @return the number of EEG channels [1-256] per box
	*/
	int EegChannelCount() const { return eegChanCount; }
	/**
	* @return the number of external channels [1-8] per box
	*/
	int ExgChannelCount() const { return exgChanCount; }
	/**
	* @return the number of auxilary channels
	*/
	int AuxChannelCount() const { return auxChanCount; }
	/**
	* @return the number of analog channels
	*/
	int AibChannelCount() const { return aibChanCount; }
	/**
	* Sets a callback for singalling the various stages of the connection via string.
	* @param fn the function to be called
	* @param logType the log type [Normal, Error, Warning]
	*/
	void SetLogCallback(std::function<void(const std::string& fn, LogType logType)>);
	
	/**
	* @return The status of the device [Idle, Initializing, Acquiring]
	*/
	Status GetStatus() { return status; }
private:
	
	typedef void* (BIOSEMI_LINKAGE* OPEN_DRIVER_ASYNC_t)(void);
	typedef int (BIOSEMI_LINKAGE* USB_WRITE_t)(void*, const unsigned char*);
	typedef int (BIOSEMI_LINKAGE* READ_MULTIPLE_SWEEPS_t)(void*, char*, int);
	typedef int (BIOSEMI_LINKAGE* READ_POINTER_t)(void*, unsigned*);
	typedef int (BIOSEMI_LINKAGE* CLOSE_DRIVER_ASYNC_t)(void*);

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

