#ifndef CAMERA_H
#define CAMERA_H

#include "SimpleMath.h"
#include "SimpleMath.inl"
using namespace DirectX;

class Camera
{
public:
	Camera();
	~Camera();
	void UpdateCamera(SHORT WKey, SHORT AKey, SHORT SKey, SHORT DKey, SHORT RKey, SHORT LMouse, SHORT Space, SHORT LCtrl, SHORT LShift);
	XMVECTOR getCamPos();
	XMVECTOR getCamForward();
	XMVECTOR getCamRight();
	XMVECTOR getCamUp();
	void setCamPos(float x, float y, float z, float w);
	void setCamPosY(float);
private:
	POINT cursorPos;
	XMVECTOR camPos;
	XMVECTOR camForward;
	XMVECTOR camRight;
	XMVECTOR camUp;
	XMVECTOR moveThisMuch;
	float moveLeftRight;
	float moveBackForwardf;
	float camYaw;
	float camPitch;
};
#endif