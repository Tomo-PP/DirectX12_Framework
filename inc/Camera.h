#pragma once

/****************************************************************
 * Include
 ****************************************************************/
#include <SimpleMath.h>
#include "define.h"


/****************************************************************
 * using namespace
 ****************************************************************/
using namespace DirectX::SimpleMath;


/****************************************************************
 * Camera�N���X
 ****************************************************************/
class Camera {

private:

	Vector3 eyePos;     /* �����̈ʒu */
	Vector3 targetPos;  /* �ڕW�_�̈ʒu */
	Vector3 upward;     /* �J�����̏�����x�N�g�� */

	float fovY;         /* �J�����̎���p */
	float Clip_near;    /* �N���b�v�����i�߁j*/
	float Clip_far;     /* �N���b�v�����i���j*/
	float aspect;       /* �E�B���h�E�̃A�X�y�N�g�� */

public:

	Camera();

	~Camera();



	/****************************************************************
	 * �J�������W�̎擾
	 ****************************************************************/
	Vector3 GetEyePos() const;

	Vector3 GetTargetPos() const;

	Vector3 GetUpWard() const;


};

