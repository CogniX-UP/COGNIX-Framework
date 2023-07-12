#include <JSON Importers/RapidJsonEyeImporter.h>

void EyeGaze::Json::RapidJsonEyeImporter::Parse(char* json)
{
	document.Parse(json);
	if (!document.HasMember(TOPIC))
		return;

	auto topic = document[TOPIC].GetString();
	auto cmp = [topic](const char* topicP) { return strcmp(topic, topicP) == 0; };

	if (cmp(FIXATION_TOPIC))
		eyeDataType = FixationType;
	else if (cmp(GAZE_BINOC_TOPIC) || cmp(GAZE_MONO_TOPIC))
		eyeDataType = GazeType;
	else if (cmp(BLINK_TOPIC))
		eyeDataType = BlinkType;
	else
		eyeDataType = Invalid;
}

EyeGaze::Data::Fixation EyeGaze::Json::RapidJsonEyeImporter::ProcessFixation()
{
	double start, duration, normPosX, normPosY, dispersion, confidence;
	
	start = document[FIXATION_START].GetDouble();
	duration = document[FIXATION_DURATION].GetDouble();
	normPosX = document[FIXATION_NORM_X].GetDouble();
	normPosY = document[FIXATION_NORM_Y].GetDouble();
	dispersion = document[DISPERSION].GetDouble();
	confidence = document[CONFIDENCE].GetDouble();

	return Fixation(start, duration, Vector2(normPosX, normPosY), dispersion, confidence);
}

EyeGaze::Data::Gaze EyeGaze::Json::RapidJsonEyeImporter::ProcessGaze()
{
	auto topic = document[TOPIC].GetString();
	double time, confidence;
	Vector2 normPos;
	Vector3 gazePoint, gazeNormalsLeft, gazeNormalsRight, eyeCenterLeft, eyeCenterRight;
	Gaze result;

	time = document[TIMESTAMP].GetDouble();
	confidence = document[CONFIDENCE].GetDouble();
	
	auto normPosParse = document[GAZE_NORM_POS].GetArray();
	normPos = Vector2(normPosParse[0].GetDouble(), normPosParse[1].GetDouble());
	
	auto gazePointParse = document[GAZE_POINT_3D].GetArray();
	gazePoint = Vector3(gazePointParse[0].GetDouble(), gazePointParse[1].GetDouble(), gazePointParse[2].GetDouble());


	if (strcmp(topic, GAZE_MONO_TOPIC) == 0) {
		auto gazeParse = document[GAZE_NORMALS_3D].GetArray();
		gazeNormalsLeft = Vector3(gazeParse[0].GetDouble(), gazeParse[1].GetDouble(), gazeParse[2].GetDouble());

		auto centerParse = document[GAZE_CENTERS_3D].GetArray();
		eyeCenterLeft = Vector3(centerParse[0].GetDouble(), centerParse[1].GetDouble(), centerParse[2].GetDouble());
		return Gaze(time, confidence, normPos, gazePoint, gazeNormalsLeft, eyeCenterLeft);
	}
	else {

		auto process = [](rapidjson::Value &part, const char* secondKey) {
			auto parse = part[secondKey].GetArray();
			return Vector3(parse[0].GetDouble(), parse[1].GetDouble(), parse[2].GetDouble());
		};

		auto& gazeParse = document[GAZE_NORMALS_3D];
		gazeNormalsLeft = process(gazeParse, "0");
		gazeNormalsRight = process(gazeParse, "1");

		auto& centerParse = document[GAZE_CENTERS_3D];
		eyeCenterLeft = process(centerParse, "0");
		eyeCenterRight = process(centerParse, "1");
		
		return Gaze(time, confidence, normPos, gazePoint, gazeNormalsLeft, gazeNormalsRight,
			eyeCenterLeft, eyeCenterRight);
	}
	
}

EyeGaze::Data::Surface* EyeGaze::Json::RapidJsonEyeImporter::ProcessSurface(std::unordered_map<std::string, Surface>& map)
{
	//the map should be initialized before entering here
	auto name = std::string(document[NAME].GetString());

	if (map.find(name) == map.end())
		return nullptr;
	auto& result = map.at(name);


	return &result;
}

EyeGaze::Data::Blink EyeGaze::Json::RapidJsonEyeImporter::ProcessBlink()
{
	double confidence, time;
	bool onset;

	confidence = document[CONFIDENCE].GetDouble();
	time = document[TIMESTAMP].GetDouble();

	auto blinkType = document[BLINK_TYPE].GetString();
	onset = strcmp(blinkType, BLINK_ONSET) == 0;

	return Blink(onset, time, confidence);
}
