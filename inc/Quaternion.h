#pragma once
/****************************************************************
 * Include
 ****************************************************************/
#include <iostream>


/****************************************************************
 * Quaternion 構造体
 ****************************************************************/
namespace QuatLib{

	struct Quaternion {

		float x;
		float y;
		float z;
		float w;

		Quaternion() = default;

		Quaternion(float _x, float _y, float _z, float _w):
			x  (_x),
			y  (_y),
			z  (_z),
			w  (_w)
		{ /* DO_NOTHING */ };
	};


	// クォータニオンの乗算
	Quaternion QuatMul(Quaternion a, Quaternion b) {

		Quaternion ans = Quaternion();
		ans.x =  a.w * b.x - a.z * b.y + a.y * b.z + a.x * b.w;
		ans.y =  a.z * b.x + a.w * b.y - a.x * b.z + a.y * b.w;
		ans.z = -a.y * b.x + a.x * b.y + a.w * b.z + a.z * b.w;
		ans.w = -a.x * b.x - a.y * b.y - a.z * b.z + a.w * b.w;
		return ans;
	}

}; // namespace QuatLib