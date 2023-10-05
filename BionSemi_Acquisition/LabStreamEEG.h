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
* The class LabStreamEEG for acquiring raw data from the BioSemi ActiveTwo, 
* rescaling and resampling if necessary and applying speficic EEG metadata.
* This class can be used to send BioSemi acquisition data through the Lab
* Streaming Layer (LSL).
*
* BioSemi acquisition with LSL
*
*/
class LabStreamEEG {
public:
	~LabStreamEEG();
	/**
	* Constructs the LSL streamer.
	*/
	LabStreamEEG() { }
	/**
	* Constructs the LSL stream, passing a specified number of channels to the consturctor.
	* @param channelCount Constructor argument for specifying the number of channels to be used.
	*/
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
	/*
	* Ensures there's a connection [physical / wireless] to the device and is ready for acquiring
	*/
	void ConnectDevice();
	/*
	* Disconnects the device
	*/
	void DisconnectDevice();
	/*
	* Stars the LSL outlet stream. 
	*/
	void StartStream();
	/*
	* Stops the LSL outlet stream.
	*/
	void StopStream();
	/*
	* Read from a configuration file
	*/
	void Load(const std::string &config, bool isFile);
	void Save(const std::string &location);
	/**
	* Sends data that has been buffered through LSL.
	*/
	void SendData(BiosemiEEG::Chunk& rawChunk, BiosemiEEG::Chunk& outChunk);
	
	const std::vector<std::string>& ActiveChannels() { return activeChannels; }
	const std::vector<std::string>& ActiveChannelTypes() { return activeChannelTypes; }
	const std::vector<std::uint32_t>& ActiveChannelIndexes() { return activeChannelIndexes; }
	//Synchronization meta-data
	const float 
		offsetMean = 0.00772,
		offsetRms = 0.000070,
		offsetMedian = 0.00772,
		offset5Centile = 0.00764,
		offset95Centile = 0.00783;

	inline static const std::string
		eegKey = "eeg",
		eegChannelCount = "count",
		exgKey = "exg";

public:
	inline static const std::string
		gKey = "general",
		compLag = "compensated_lag",
		causalCorr = "causal_correction";
private:

	std::unique_ptr<lsl::stream_outlet> outlet;
	lsl::stream_info streamInfo;
	BiosemiEEG eeg;
	BiosemiStreamSetting streamSetting;

	//Current set up for acquiring
	std::vector<std::string> activeChannels;
	std::vector<std::string> activeChannelTypes;
	std::vector<uint32_t> activeChannelIndexes;
};