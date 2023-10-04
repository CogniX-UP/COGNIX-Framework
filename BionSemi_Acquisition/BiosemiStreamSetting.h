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

	//json keywords
public:
	inline static const std::string
		eegKey = "eeg",
		eegChannelCount = "count",
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
	const std::string& StreamName() { return streamName; }
	void SetStreamName(std::string name) { streamName = name; }
	void Load(const std::string& content, bool isFile);
	void Save(const std::string& filePath);
	std::vector<bool>& GetExgs() { return exgs; }
	std::map<std::string, std::string> &GetBiosemiToStream() { return biosemiToStream; }
	int interval = 50;
	int compensatedLag = 0;
	int causalCorrection = 0;
	int channelCount = 32;
private:
	//These represent json data
	std::string streamName = "biosemi";
	std::vector<bool> exgs { false, false, false, false, false, false, false, false };
	std::map<std::string, std::string> biosemiToStream;
};

