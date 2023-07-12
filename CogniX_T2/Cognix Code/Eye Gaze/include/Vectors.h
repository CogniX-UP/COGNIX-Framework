#pragma once
namespace EyeGaze {
	namespace Data {
		struct Vector2 {
			double x, y;
			Vector2() {

			};
			Vector2(double x, double y) {
				this->x = x;
				this->y = y;
			};
		};
		struct Vector3 {
			double x, y, z;
			Vector3() {

			};
			Vector3(double x, double y, double z) {
				this->x = x;
				this->y = y;
				this->z = z;
			};
		};
	}
}