#pragma once
#include <BaseJsonEyeImporter.h>
#include "rapidjson/document.h"
#include <string>

namespace EyeGaze {
	namespace Json {
		class RapidJsonEyeImporter: public BaseJsonEyeImporter {
		private:
			rapidjson::Document document;
		public:
			virtual void Parse(char* json) override;
			virtual Fixation ProcessFixation() override;
			virtual Gaze ProcessGaze() override;
			virtual Surface ProcessSurface() override;
			virtual Blink ProcessBlink() override;
		};
	}
}