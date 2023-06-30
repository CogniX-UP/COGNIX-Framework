#pragma once
namespace EyeGaze {
	namespace Data {
		class Blink {
		public:
			Blink(bool onset, double time, double confidence) {
				this->onset = onset;
				this->time = time;
				this->confidence = confidence;
			};
		private:
			bool onset;
			double time;
			double confidence;
		public:
			bool IsOnset() { return onset; }
			double GetTime() { return time; }
			double GetConfidence() { return confidence; }
		};
	}
}