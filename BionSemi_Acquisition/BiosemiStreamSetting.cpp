#include "BiosemiStreamSetting.h"
#include <stdexcept>
#include "BiosemiEEG.h"
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <iostream>
#include <fstream>
#include <rapidjson/prettywriter.h>

#ifdef _WIN32
const char* frMode = "rb";
const char* fwMode = "wb";
#elif
const char* frMode = "r";
const char* fwMode = "w";
#endif

#define key(x)  rapidjson::GenericStringRef(x.c_str())


std::vector<bool> BiosemiStreamSetting::ChannelMask(BiosemiEEG &eeg) {
	auto res = std::vector<bool>(eeg.AllChannelCount());
	// TODO (DONT) Ignore the trigger and sync for now
	int c = 0;
	for (int i = c; i < eeg.SyncChannelCount(); i++)
		res[i] = false;
	c += eeg.SyncChannelCount();
	for (int i = c; i < eeg.TriggerChannelCount() + c; i++)
		res[i] = false;
	c += eeg.TriggerChannelCount();
	// We'll also ignore multi box for now
	//Channels
	int eegCount = 0;
	for (int i = c; i < eeg.EegChannelCount() + c; i++) {
		if (eegCount >= eegChannelCount)
			break;
		eegCount++;
		res[i] = true;
	}
	c += eeg.EegChannelCount();
	//EXG
	auto &exgs = GetExgs();
	for (int i = c; i < eeg.ExgChannelCount() + c; i++) {
		res[i] = exgs[i - c];
	}
	c += eeg.ExgChannelCount();
	// TODO (DONT) Ignore the rest
	return res;
}

void BiosemiStreamSetting::Load(const std::string &config, bool isFile)
{
	//Parse the doc
	rapidjson::Document doc;
	if (isFile)
	{
		FILE* file = fopen(config.c_str(), frMode);
		if (!file) {
			throw std::runtime_error("Could not open config FILE at: " + config);
		}

		auto buffer = std::make_unique<char[]>(65536);
		rapidjson::FileReadStream reader(file, buffer.get(), sizeof(buffer.get()));
		
		doc.ParseStream(reader);
		if (doc.HasParseError())
			throw std::exception("Parse Error");

		fclose(file);
	}
	else
	{
		doc.Parse(config.c_str());
	}

	//Retrieve data from the json
	auto& gSetting = doc[gKey.c_str()];

	if (!gSetting.IsNull()) {
		if (gSetting.HasMember(intervalKey.c_str()))
			interval = gSetting[intervalKey.c_str()].GetInt();
		if (gSetting.HasMember(causalCorrKey.c_str()))
			causalCorrection = gSetting[causalCorrKey.c_str()].GetInt();
		if (gSetting.HasMember(compLagKey.c_str()))
			compensatedLag = gSetting[compLagKey.c_str()].GetInt();
		if (gSetting.HasMember(streamNameKey.c_str()))
			streamName = std::string(gSetting[streamNameKey.c_str()].GetString());
	}

	auto& eegSetting = doc[eegKey.c_str()];
	
	if (!eegSetting.IsNull()) {	
		auto& exgSetting = eegSetting[exgKey.c_str()];
		if (!exgSetting.IsNull()) 
		{
			//EXGs	
			for (int i = 0; i < exgNames.size(); i++) {
				auto& name = exgNames[i];
				auto& valueObj = exgSetting[name.c_str()];
				
				if (!valueObj.IsNull()) {
					exgs[i] = valueObj.GetBool();
				}
			}

		}

		auto& channelSetting = eegSetting[chanKey.c_str()];
		if (!channelSetting.IsNull()) {
			eegChannelCount = channelSetting[chanCountKey.c_str()].GetInt();
			//EEG labels for non-daisy chained boxes
			auto& biosemiLabels = GetBiosemiLabels();
			biosemiToStream.clear();
			for (auto& label : biosemiLabels) {
				auto& rename = channelSetting[label.c_str()];
				if (rename.IsNull())
					continue;

				biosemiToStream.insert_or_assign(label, rename.GetString());
			}
		}
	}
	

}

void BiosemiStreamSetting::Save(const std::string& filePath)
{
	FILE* configFile = fopen(filePath.c_str(), fwMode);

	//Open the previous file and read it
	if (!configFile) {
		//perhaps create a file here later
		throw std::runtime_error("Could not open config FILE at: " + filePath);
	}

	auto buffer = std::make_unique<char[]>(65536);
	//Open the file again to write to it
	rapidjson::FileWriteStream writeStream(configFile, buffer.get(), sizeof(buffer.get()));
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(writeStream);
	rapidjson::Document doc;
	
	doc.SetObject();
	auto& alloc = doc.GetAllocator();

	//We'll create the file from scratch. Perhaps later we might change it for certain parts.
	//Doubt it though

	rapidjson::Value generalValue(rapidjson::kObjectType);
	generalValue.AddMember(key(streamNameKey), key(streamName), alloc);
	generalValue.AddMember(key(intervalKey), interval, alloc);
	generalValue.AddMember(key(compLagKey), compensatedLag, alloc);
	generalValue.AddMember(key(causalCorrKey), causalCorrection, alloc);
	doc.AddMember(key(gKey), generalValue, alloc);

	rapidjson::Value exgValue(rapidjson::kObjectType);
	for (int i = 0; i < exgs.size(); i++) {
		bool value = exgs[i];
		exgValue.AddMember(key(exgNames[i]), value, alloc);
	}

	rapidjson::Value chanValue(rapidjson::kObjectType);
	chanValue.AddMember(key(chanCountKey), eegChannelCount, alloc);
	for (auto& pair : biosemiToStream) {
		chanValue.AddMember(key(pair.first), key(pair.second), alloc);
	}

	rapidjson::Value eegValue(rapidjson::kObjectType);
	eegValue.AddMember(key(exgKey), exgValue, alloc);
	eegValue.AddMember(key(chanKey), chanValue, alloc);
	doc.AddMember(key(eegKey), eegValue, alloc);

	doc.Accept(writer);
	fclose(configFile);
}
