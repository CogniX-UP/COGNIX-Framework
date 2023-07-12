#pragma once
#include <Fixation.h>
#include <Surface.h>
#include <Blink.h>
#include <Gaze.h>
#include <EyeDataType.h>
#include <unordered_map>
#include <string>
namespace EyeGaze {
	namespace Json {
		using namespace EyeGaze::Data;

		class BaseJsonEyeImporter {
		protected:
			char* json;
			EyeDataType eyeDataType;
		public:
			EyeDataType GetDataType() { return eyeDataType; }
			virtual void Parse(char* json) = 0;
			virtual Fixation ProcessFixation() = 0;
			virtual Gaze ProcessGaze() = 0;
			virtual Surface* ProcessSurface(std::unordered_map<std::string, Surface> &map) = 0;
			virtual Blink ProcessBlink() = 0;
		};

		//Topics
		const char* TOPIC = "topic";
		const char* CONFIDENCE = "confidence";
		const char* METHOD = "method";
		const char* TIMESTAMP = "timestamp";
		const char* DISPERSION = "dispersion";
		//Pupil
		const char* EYE_ID = "id";
		const char* EYE_ID_LEFT = "0";
		const char* EYE_ID_RIGHT = "1";
		const char* EYE_LEFT_TOPIC = "pupil.0";
		const char* EYE_RIGHT_TOPIC = "pupil.1";

		//Gaze
		const char* GAZE_MONO_TOPIC = "gaze.3d.1.";
		const char* GAZE_BINOC_TOPIC = "gaze.3d.01.";
		const char* GAZE_NORM_POS = "norm_pos";
		const char* GAZE_POINT_3D = "gaze_point_3d";
		const char* GAZE_NORMALS_3D = "gaze_normals_3d";
		const char* GAZE_CENTERS_3D = "gaze_centers_3d";

		//Fixation
		const char* FIXATION_TOPIC = "fixation";
		const char* FIXATION_START = "start_timestamp";
		const char* FIXATION_DURATION = "duration";
		const char* FIXATION_NORM_X = "norm_pos_x";
		const char* FIXATION_NORM_Y = "norm_pos_y";

		//Blink
		const char* BLINK_TOPIC = "blink";
		const char* BLINK_TYPE = "type";
		const char* BLINK_ONSET = "onset";
		const char* BLINK_OFFSET = "offset";

		//Surface
		const char* NAME = "name";
		const char* SURF_GAZE_TOPIC = "gaze_on_surfaces";
		const char* SURF_FIX_TOPIC = "fixations_on_surfaces";
	}

}