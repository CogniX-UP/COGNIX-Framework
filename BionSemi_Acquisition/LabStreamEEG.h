/**
 * @file EEG/LabStreamEEG.h
 * @author Georgios Papadoulis
 *
 * BioSemi acquisition with LSL
 *
 */
#pragma once
#include <lsl_cpp.h>
#include "BiosemiEEG.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <rapidjson/document.h>
#include <unordered_map>
#include "BiosemiStreamSetting.h"
/**
* The class LabStreamEEG for acquiring raw data from the BioSemi ActiveTwo 
* and applying speficic EEG metadata.
* This class can be used to send BioSemi acquisition data through the Lab
* Streaming Layer (LSL). It is structured in an abstract way and separate from
* the BioSemi device, in case more devices are to be added in the future that 
* may use the same LSL interface.
*
*/
class LabStreamEEG {
public:
	/**
	* Releases the LSL streamer.
	*/
	~LabStreamEEG();
	/**
	* Constructs the LSL streamer.
	*/
	LabStreamEEG() { }
	/**
	* Returns the stream info, a collection of metadata describing the BioSemi ActiveTwo acquisition.
	*/
	lsl::stream_info GetStreamInfo() { return streamInfo; }
	/**
	* Returns the underlying BioSemi low level API interface.
	*/
	BiosemiEEG& GetBiosemiInterface() { return eeg; }
	/**
	* Gets the Stream Setting parameters
	*/
	BiosemiStreamSetting& GetStreamSetting() { return streamSetting; }
	/**
	* Ensures there's a connection [physical] to the device and is ready for acquiring.
	* It throws an std::runtime_error if the connection to the device isn't established or
	* if the device is already acquiring data.
	*/
	void ConnectDevice() throw(std::runtime_error);
	/**
	* Terminates acquisition from the underlying device.
	*/
	void DisconnectDevice();
	/*
	* Starts the LSL outlet stream. Metadata is broadcasted in UDP bursts and data is broadcasted via TCP.
	*/
	void StartStream();
	/**
	* Terminates the LSL outlet stream.
	*/
	void StopStream();
	/**
	* Reads from a configuration file. For the BioSemi Active Two, the file is in JSON format.
	* @param config The configuration as a string, or the file location where the configuration is.
	* @param isFile A boolean indicating whether config is the configuration or the configuration location
	*/
	void Load(const std::string& config, bool isFile);
	/**
	* Saves the data as a configuration file, in the format implemented for this streamer.
	* @param location The location where the file is to be stored, typically relative to the application.
	*/
	void Save(const std::string& location);
	/**
	* Sends data that has been buffered through LSL.
	* @param rawChunk External buffer that stores the raw eeg data, mostly for viewing purposes.
	* @param outChunk External buffer that stores the 
	*/
	void SendData(BiosemiEEG::Chunk& rawChunk, BiosemiEEG::Chunk& outChunk);
	
	/**
	* @return The active channel count we're sending through LSL.
	*/
	const std::vector<std::string>& ActiveChannels() { return activeChannels; }
	/**
	* @return The active channel types we're sending through LSL (i.e. EEG)
	*/
	const std::vector<std::string>& ActiveChannelTypes() { return activeChannelTypes; }
	/**
	* @return The indexes of the active channels in relation to the total channel vector.
	*/
	const std::vector<std::uint32_t>& ActiveChannelIndexes() { return activeChannelIndexes; }
	
	/**
	* Synchronization meta-data, as found in [website]https://sccn.ucsd.edu/download/mgrivich/LSL_Validation.html
	*/
	const float 
		offsetMean = 0.00772,
		offsetRms = 0.000070,
		offsetMedian = 0.00772,
		offset5Centile = 0.00764,
		offset95Centile = 0.00783;

	/**
	* Keys for configuration loading.
	*/
	inline static const std::string
		eegKey = "eeg",
		eegChannelCount = "count",
		exgKey = "exg";

public:
	/**
	* Keys for appending meta-data.
	*/
	inline static const std::string
		gKey = "general",
		compLag = "compensated_lag",
		causalCorr = "causal_correction";
private:
	/**
	* A unique pointer to the underlying stream outlet.
	*/
	std::unique_ptr<lsl::stream_outlet> outlet;
	/**
	* The stream info used to construct the outlet.
	*/
	lsl::stream_info streamInfo;
	/**
	* The underlying BioSemi EEG acquisition class.
	*/
	BiosemiEEG eeg;
	/**
	* The stream setting, that can be loaded from or saved to disk.
	*/
	BiosemiStreamSetting streamSetting;

	//Current set up for acquiring
	std::vector<std::string> activeChannels;
	std::vector<std::string> activeChannelTypes;
	std::vector<uint32_t> activeChannelIndexes;
};