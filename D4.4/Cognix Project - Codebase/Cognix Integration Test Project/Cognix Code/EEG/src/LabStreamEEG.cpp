#include "LabStreamEEG.h"

LabStreamEEG::LabStreamEEG(int channelCount)  {
	//basic data for stream to work
	streamInfo = lsl::stream_info("BioSemi", "EEG", channelCount, eeg.srate(), lsl::cf_double64, "EEG_BIOSEMI");
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
	
	//This may be needed later
	outlet = new lsl::stream_outlet(streamInfo);
}
LabStreamEEG::~LabStreamEEG()
{
	delete outlet;
}

void LabStreamEEG::AppendChannelMetadata(const std::vector<std::string>& channels, const std::vector<std::string>& types, const std::unordered_map<std::string, std::vector<std::string>>& locmap)
{
	auto chanInfo = streamInfo.desc().append_child("channels");
	for (int i = 0; i < channels.size(); i++) {
		auto channel = chanInfo.append_child("channel");
		channel.append_child_value("label", channels[i]);
		channel.append_child_value("type", types[i]);
		channel.append_child_value("unit", "microvolts");
		auto hardware = channel.append_child("hardware");
		hardware.append_child_value("manufacturer", "BioSemi");

		if (types[i] == "EEG") {
			hardware.append_child_value("coupling", "Gel");
			hardware.append_child_value("material", "Ag-AgCl");
			hardware.append_child_value("surface", "Pin");
		}

		if (locmap.count(channels[i])) {
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
	}
}

void LabStreamEEG::AppendRefMetadata(const std::vector<std::string>& refNames)
{
	auto reference = streamInfo.desc().append_child("reference");
	reference.append_child_value("subtracted", "No");
	for (auto ref : refNames)
		reference.append_child_value("label", ref);
}

void LabStreamEEG::AppendAcquisitionMetadata(float compensatedLag, float causalCorrection)
{
	auto acq = streamInfo.desc().append_child("acquisition");
	acq.append_child_value("manufacturer", "BioSemi")
		.append_child_value("model", eeg.IsMk2() ? "ActiveTwo Mk2" : "ActiveTwo Mk1")
		.append_child_value("precision", "24")
		.append_child_value("compensated_lag", std::to_string(compensatedLag))
		.append_child_value("causal_correction", std::to_string(causalCorrection));
}

void LabStreamEEG::SendData()
{
	BiosemiEEG::Chunk rawChunk, scaledChunk;

	//Chunk from the device [#insamples x #channels]
	eeg.GetChunk(rawChunk);
	//TODO change this later for specific channels

	int outChannels = eeg.AllChannelCount();
	int inSamples = rawChunk.size();

	if (inSamples <= 0)
		return;

	
	//Scale to microvolts
	scaledChunk.resize(rawChunk.size());
	for (int i = 0; i < outChannels; i++) {
		scaledChunk[i].resize(inSamples);
		
		bool isTrigger = false; //TODO add a way to find if its the trigger
		float scaleFactor = isTrigger ? 256 : 256 * 0.03125;
		for (int j = 0; i < inSamples; j++) {
			scaledChunk[i][j] = rawChunk[i][j] / scaleFactor;
		}

	}

	//TODO resample

	outlet->push_chunk(scaledChunk, lsl::local_clock() - compensatedLag);
}

