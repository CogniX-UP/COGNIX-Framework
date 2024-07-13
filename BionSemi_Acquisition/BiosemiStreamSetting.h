/**
 * @file EEG/BiosemiStreamSetting.h
 * @author Georgios Papadoulis
 *
 * BioSemi acquisition with LSL
 *
 */
#pragma once
#include <string>
#include <vector>
#include <map>
#include "BiosemiEEG.h"

 /**
 * The class BiosemiStreamSetting for acquiring raw data from the BioSemi ActiveTwo,
 * represents basic settings that a single box from Biosemi MK2 can load. It does not take
 * into daisy-chained configurations or MK1 configurations. The data can be stored and restored
 * from a json file using rapidjson.
 *
 * BioSemi acquisition with LSL
 *
 */
class BiosemiStreamSetting
{

public:
	inline static std::vector<std::string> biosemiLabels;
	static const std::vector<std::string>& GetBiosemiLabels() 
	{

		if (biosemiLabels.size() == 0) {
			for (int k = 1; k <= 256; k++) {
				//BioSemi might have A-H, hence we use index k to create the A1-H32
				std::string tmp = "A";
				tmp[0] = 'A' + (k - 1) / 32;
				biosemiLabels.push_back(std::string(tmp) += std::to_string(1 + (k - 1) % 32));
			}
		}
		return biosemiLabels;
	}

public:
	/**
	* Keys for easier access to the JSON.
	*/
	inline static const std::string
		eegKey = "eeg",
		exgKey = "exg",
		gKey = "general",
		compLagKey = "compensated_lag",
		causalCorrKey = "causal_correction",
		intervalKey = "interval",
		chanKey = "channels",
		chanCountKey = "count",
		streamNameKey = "stream_name";
	inline static const
		std::vector<std::string> exgNames {"EX1", "EX2", "EX3", "EX4", "EX5", "EX6", "EX7", "EX8"};

public:
	/**
	* @return The unique stream name for this acquisition.
	*/
	const std::string& StreamName() { return streamName; }
	/**
	* Sets the stream acquisition name
	* @param name The acquisition name
	*/
	void SetStreamName(std::string name) { streamName = name; }
	/**
	* Loads the contents of a config file to this stream setting.
	* @param content The content as text or the location to the config file.
	* @param isFile Whether it is text content or location file.
	*/
	void Load(const std::string& content, bool isFile);
	/**
	* Saves this BioSemi stream setting to the specific location.
	*/
	void Save(const std::string& filePath);
	/**
	* Indicates which of the (EX1-EX8) should be sent through LSL.
	* @return The external (EX1-EX8) setting.
	*/
	std::vector<bool>& GetExgs() { return exgs; }
	/**
	* BioSemi has a 256 available channels, typically named A1-H32. This returns a mapping of
	* (A1-H32) -> (other_names), since typically we will have 32 channels and we want them named
	* per the 10-20 system. (281 channels with the peripherals available)
	* 
	* @return A mapping of the local channel names to custom names set by the user.
	*/
	std::map<std::string, std::string> &GetBiosemiToStream() { return biosemiToStream; }

	/*
	* Creates a mask of which channels to send to the eeg, ordered as originally received
	* through their USB interface.
	*/
	std::vector<bool> ChannelMask(BiosemiEEG &eeg);
	/**
	* The time waited for the next chunk to be sent.
	*/
	int interval = 50;
	/**
	* If any constant lag is introduced in the process
	*/
	int compensatedLag = 0;
	/**
	* If any correction is needed due to any filtering.
	*/
	int causalCorrection = 0;
	/**
	* The channels count of EEG channels to be sent (A1-H32), each label having 32 channels.
	* The values it can take is 32, 64, 128, 256
	*/
	int eegChannelCount = 32;
private:
	std::string streamName = "biosemi";
	std::vector<bool> exgs { false, false, false, false, false, false, false, false };
	std::map<std::string, std::string> biosemiToStream;
};

