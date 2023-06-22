#pragma once
#include "Vectors.h"
namespace EyeGaze {
	namespace Data {
		class Fixation {
		public:
			Fixation(double start, double duration, Vector2 normPos, double dispersion,
				double confidence) {
				this->start = start;
				this->duration = duration;
				this->normPos = normPos;
				this->dispersion = dispersion;
				this->confidence = confidence;
			}
		private:
			double start;
			double duration;
			Vector2 normPos;
			double dispersion;
			double confidence;
		public:
			//start time in ms
			double GetStartTime() const { return start; }
			//duration in ms
			double GetDuration() const { return duration; }
			Vector2 GetNormPos() const { return normPos; }
			double GetDispersion() const { return dispersion; }
			//how accurate the detection was, 0-1
			double GetConfidence() const { return confidence; }
		};
	}
}