#include "LabStreamEEG.h"
#include <rapidjson/filereadstream.h>
#include <cstdio>
#include <rapidjson/document.h>
//-_-
#ifdef _WIN32
const char* fMode = "rb";
#elif
const char* fMode = "r";
#endif

LabStreamEEG::~LabStreamEEG()
{

}

void LabStreamEEG::ConnectDevice() {
	if (eeg.GetStatus() != BiosemiEEG::Idle)
		throw std::runtime_error("Biosemi Device already initialized");

	try {
		//Connect the amplifier
		eeg.ConnectAmplifier();

		//basic data for stream to work
		streamInfo = lsl::stream_info("BioSemi", "EEG", streamSetting.channelCount, eeg.SampleRate(), lsl::cf_double64, streamSetting.StreamName());
		streamInfo.desc().append_child("amplifier")
			.append_child("settings")
			.append_child_value("speedmode", std::to_string(eeg.SpeedMode()));
		//synchronization meta-data, taken from lsl
		streamInfo.desc().append_child("synchronization")
			.append_child_value("offset_mean", std::to_string(LabStreamEEG::offsetMean))
			.append_child_value("offset_rms", std::to_string(LabStreamEEG::offsetRms))
			.append_child_value("offset_median", std::to_string(LabStreamEEG::offsetMedian))
			.append_child_value("offset_5_centile", std::to_string(LabStreamEEG::offset5Centile))
			.append_child_value("offset_95_centile", std::to_string(LabStreamEEG::offset95Centile));

		//Order <Label, Type> "Sync, Sync" -> "Trig, Trigger" -> "A1-H32, EEG", "EX, EXG", "AUX, AUX", "AIB, Analog"
		//Our BioSemi json needs to have a specific structure, or else it won't work

		auto chanInfo = streamInfo.desc().append_child("channels");

		activeChannels.clear();
		activeChannelTypes.clear();
		activeChannelIndexes.clear();

		auto channelCount = streamSetting.channelCount;
		//Ignore trigger for now
		int currentEegCount = 0;
		int currentExgCount = 0;

		//Order <Label, Type> "Sync, Sync" -> "Trig, Trigger" -> "A1-H32, EEG", "EX, EXG", "AUX, AUX", "AIB, Analog"
		//Apply channels 
		for (int i = 0; i < eeg.AllChannelCount(); i++) {

			auto& type = eeg.ChannelTypes()[i];
			//Ignore everything but the eeg and the exg
			if (type != BiosemiEEG::eegType && type != BiosemiEEG::exgType)
				continue;

			//These are typical for all
			auto channel = chanInfo.append_child("channel");
			channel.append_child_value("type", type);
			channel.append_child_value("unit", "microvolts");
			auto hardware = channel.append_child("hardware");
			hardware.append_child_value("manufacturer", "BioSemi");

			//current biosemi label
			std::string chanLabel = eeg.ChannelLabels()[i];
			if (type == BiosemiEEG::eegType)
			{
				//Apply channel constraint
				if (currentEegCount >= channelCount)
					continue;
				currentEegCount++;

				//Rename if there is another name in the configuration
				auto& bioMap = streamSetting.GetBiosemiToStream();
				if (bioMap.find(chanLabel.c_str()) != bioMap.end())
					chanLabel = std::string(bioMap[chanLabel.c_str()]);

				hardware.append_child_value("coupling", "Gel");
				hardware.append_child_value("material", "Ag-AgCl");
				hardware.append_child_value("surface", "Pin");
			}
			else if (type == BiosemiEEG::exgType)
			{
				//This whole section acts as a guard for the EXG
				bool sendExg = streamSetting.GetExgs()[currentExgCount];
				currentExgCount++;
				continue;
			}

			channel.append_child_value("label", chanLabel);

			activeChannels.push_back(chanLabel);
			activeChannelIndexes.push_back(i);
			activeChannelTypes.push_back(type);

			//Acquisition metadata
			auto acq = streamInfo.desc().append_child("acquisition");
			acq.append_child_value("manufacturer", "BioSemi")
				.append_child_value("model", eeg.IsMk2() ? "ActiveTwo Mk2" : "ActiveTwo Mk1")
				.append_child_value("precision", "24")
				.append_child_value("compensated_lag", std::to_string(streamSetting.compensatedLag))
				.append_child_value("causal_correction", std::to_string(streamSetting.causalCorrection));

			//Ref Metadata - Will leave that for later, ref isn't used nowadays
			//auto reference = streamInfo.desc().append_child("reference");
			//reference.append_child_value("subtracted", "No");
			//for (auto ref : refNames)
			//	reference.append_child_value("label", ref);
		}
	}
	catch (const std::exception& e) {
		throw e;
	}
}
void LabStreamEEG::DisconnectDevice() {
	if (eeg.GetStatus() != BiosemiEEG::Idle)
		eeg.DisconnectAmplifier();
}

void LabStreamEEG::StartStream() {
	if (eeg.GetStatus() != BiosemiEEG::Acquiring)
		throw std::runtime_error("The BioSemi device has not been initialized");

	outlet.reset(new lsl::stream_outlet(streamInfo));
}
void LabStreamEEG::StopStream() {
	auto pointer = outlet.release();
	if (pointer)
		delete pointer;
}

/*
* THIS IS USED FOR ELECTRODE LOCATIONS, WILL CHECK IF ADDED LATER
*
* const std::vector<std::string>& types, const std::unordered_map<std::string, std::vector<std::string>>& locmap
* if (locmap.count(channels[i])) {
			//TODO error checks
			auto channelLoc = locmap.at(channels[i]);
			auto x = channelLoc[0];
			auto y = channelLoc[1];
			auto z = channelLoc[2];
			channel.append_child("location")
				.append_child_value("X", x)
				.append_child_value("Y", y)
				.append_child_value("Z", z);
		}
*/

void LabStreamEEG::Load(const std::string &config, bool isFile) {

	try {
		streamSetting.Load(config, isFile);
	}
	catch (const std::exception& e) {
		throw e;
	}
	
}
void LabStreamEEG::Save(const std::string& location) {
	try {
		streamSetting.Save(location);
	}
	catch (const std::exception &e){
		throw e;
	}
}

void LabStreamEEG::SendData(BiosemiEEG::Chunk &rawChunk, BiosemiEEG::Chunk &outChunk)
{
	if (eeg.GetStatus() != BiosemiEEG::Acquiring)
		return;

	rawChunk.clear();
	outChunk.clear();
	//Chunk from the device [#insamples x #channels] but lsls wants [#channels x #outsamples]
	try {
		eeg.GetChunk(rawChunk);
	}
	catch (const std::exception& e) {
		throw e;
	}

	int outChannels = activeChannels.size();
	int inSamples = rawChunk.size();
	if (inSamples <= 0)
		return;

	outChunk.resize(inSamples);

	//Resize the internal channel vectors
	for (int i = 0; i < inSamples; i++) {
		outChunk[i].resize(outChannels);
	}

	//Apply the scaled data to the output
	for (int i = 0; i < outChannels; i++) {
		int index = activeChannelIndexes[i];
		auto& type = activeChannelTypes[index];
		auto& label = activeChannels[index];
		
		bool isTrigger = type == BiosemiEEG::triggerType;
		//float scaleFactor = isTrigger ? 256 : 256 * 0.03125; // scale to microvolts, in some boxes it may be div 4
		float scaleFactor = isTrigger ? 256 : 256 * 32;
		for (int j = 0; j < inSamples; j++) {
			outChunk[j][i] = rawChunk[j][index] / scaleFactor;
		}
	}

	//Do not resample before sending, may cause noise to alias in the lower frequencies!
	outlet->push_chunk(outChunk, lsl::local_clock() - streamSetting.compensatedLag);
}

