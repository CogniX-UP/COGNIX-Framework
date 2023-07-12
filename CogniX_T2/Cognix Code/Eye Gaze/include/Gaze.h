#pragma once
#include "Vectors.h"
namespace EyeGaze {
	namespace Data {
		class Gaze {
			enum GazeType{Monocular, Binocular};
		public:
			Gaze() {};
			Gaze(double time, double confidence, Vector2 normPos, Vector3 gazePoint, Vector3 gazeNormals, Vector3 eyeCenter) {
				this->time = time;
				this->confidence = confidence;
				this->gazeType = Monocular;
				this->normPos = normPos;
				this->gazePoint = gazePoint;
				this->gazeNormalsLeft = gazeNormals;
				this->eyeCenterLeft = eyeCenter;
			}
			Gaze(double time, double confidence, Vector2 normPos, Vector3 gazePoint, Vector3 gazeNormalsLeft, Vector3 gazeNormalsRight, Vector3 eyeCenterLeft,
				Vector3 eyeCenterRight) {
				this->time = time;
				this->confidence = confidence;
				this->gazeType = Binocular;
				this->normPos = normPos;
				this->gazePoint = gazePoint;
				this->gazeNormalsLeft = gazeNormalsLeft;
				this->gazeNormalsRight = gazeNormalsRight;
				this->eyeCenterLeft = eyeCenterLeft;
				this->eyeCenterRight = eyeCenterRight;
			}
		private:
			double time;
			double confidence;
			GazeType gazeType;
			Vector2 normPos;
			Vector3 gazePoint;
			Vector3 gazeNormalsLeft;
			Vector3 gazeNormalsRight;
			Vector3 eyeCenterLeft;
			Vector3 eyeCenterRight;
		public:
			double GetTime() { return time; }
			double GetConfidence() { return confidence; }
			GazeType GetGazeType() { return gazeType; }
			Vector2 GetNormPos() { return normPos; }
			Vector3 GetGazePoint() { return gazePoint; }
			//monocular gaze normals if gaze type is monocular
			Vector3 GetGazeNorm() { return gazeNormalsLeft; }
			Vector3 GetGazeNormLeft() { return gazeNormalsLeft; }
			Vector3 GetGazeNormRight() { return gazeNormalsRight; }
			//monocular eye center if gaze type is monocular
			Vector3 GetEyeCenter() { return eyeCenterLeft; }
			Vector3 GetEyeCenterLeft() { return eyeCenterLeft; }
			Vector3 GetEyeCenterRight() { return eyeCenterRight; }
		};
	}
}