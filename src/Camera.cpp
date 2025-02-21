
/****************************************************************
 * Include
 ****************************************************************/
#include "Camera.h"


Camera::Camera():
	eyePos    (Vector3(10.0f, 0.0f, 0.0f)),
	targetPos (Vector3(0.0f, 0.0f, 0.0f)),
	upward    (Vector3(0.0f, 1.0f, 0.0f)),
	fovY      (45.0f),
	Clip_near (10.0f),
	Clip_far  (1000.0f),
	aspect    (WINDOW_WIDTH / WINDOW_HEIGHT)
{ /* DO_NOTHING */ }


Camera::~Camera()
{ /* DO_NOTHING */ }



Vector3 Camera::GetEyePos() const{

	return eyePos;
}


Vector3 Camera::GetTargetPos() const{

	return targetPos;
}


Vector3 Camera::GetUpWard() const{

	return upward;
}