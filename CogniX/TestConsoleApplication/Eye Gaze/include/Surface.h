#pragma once
#include <vector>
#include <Gaze.h>
#include <Fixation.h>
namespace EyeGaze {
	namespace Data {
		class Surface {
		public:
			Surface(char* name, double detectionTime) {
				this->name = name;
				this->detectionTime = detectionTime;
			};
		private:
			//to be changed to string view
			char* name;
			double detectionTime;
			std::vector<Gaze> gazeList;
			std::vector<Fixation> fixationList;
		public:
			const char* GetName() { return name; }
			double GetDetectionTime() { return detectionTime; }
			const std::vector<Gaze>& GetGazeList() { return gazeList; }
			const std::vector<Fixation>& GetFixationList() { return fixationList; }
		};
	}
}