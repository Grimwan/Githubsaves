#include "Camera.h"

Camera::Camera()
{
	camPos = XMVectorSet(0, 0, 5, 1);
	camForward = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	camRight = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	moveLeftRight = 0.0f;
	moveBackForwardf = 0.0f;
	camYaw = 0.0f;
	camPitch = 0.0f;
	moveThisMuch = XMVectorReplicate(0.001);
}

Camera::~Camera()
{

}

void Camera::UpdateCamera(SHORT WKey, SHORT AKey, SHORT SKey, SHORT DKey, SHORT RKey, SHORT LMouse, SHORT Space, SHORT LCtrl, SHORT LShift)
{
	if (LShift)
		moveThisMuch = XMVectorReplicate(0.5);
	if (!LShift)
		moveThisMuch = XMVectorReplicate(0.1);
	if (WKey)
		camPos = XMVectorMultiplyAdd(moveThisMuch, camForward, camPos);
	if (AKey)
		camPos = XMVectorMultiplyAdd(-moveThisMuch, camRight, camPos);
	if (SKey)
		camPos = XMVectorMultiplyAdd(-moveThisMuch, camForward, camPos);
	if (DKey)
		camPos = XMVectorMultiplyAdd(moveThisMuch, camRight, camPos);
	if (RKey)
	{
		camPos = XMVectorSet(0, 0, 5, 1);
		camForward = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		camRight = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
		camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	}
	if (Space)
		camPos = XMVectorMultiplyAdd(moveThisMuch, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), camPos);
	if (LCtrl)
		camPos = XMVectorMultiplyAdd(-moveThisMuch, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), camPos);
	if (LMouse)
	{
		POINT newPos;

		GetCursorPos(&newPos);
		// handle left and right
		int changeX = cursorPos.x - newPos.x;
		float rot = 0.0;
		if (changeX < 0) rot = 0.01;
		else if (changeX > 0) rot = -0.01;
		if (rot != 0.0)
		{
			XMMATRIX rotMat = XMMatrixRotationY(rot);
			camRight = XMVector3TransformNormal(camRight, rotMat);
			camForward = XMVector3TransformNormal(camForward, rotMat);
			camUp = XMVector3TransformNormal(camUp, rotMat);
		}
		// handle up and down
		int changeY = cursorPos.y - newPos.y;
		rot = 0.0;
		if (changeY < 0) rot = 0.01;
		else if (changeY > 0) rot = -0.01;
		if (rot != 0.0)
		{
			XMMATRIX rotMat = XMMatrixRotationAxis(camRight, rot);
			camForward = XMVector3TransformNormal(camForward, rotMat);
			camUp = XMVector3TransformNormal(camUp, rotMat);
		}
		cursorPos = newPos;
	}
}
void Camera::setCamPosY(float yValue)
{
	XMFLOAT4 temp;
	XMStoreFloat4(&temp, camPos);
	temp.y = yValue;
	camPos = XMLoadFloat4(&temp);
}

XMVECTOR Camera::getCamPos()
{
	return camPos;
}

XMVECTOR Camera::getCamForward()
{
	return camForward;
}

XMVECTOR Camera::getCamRight()
{
	return camRight;
}

XMVECTOR Camera::getCamUp()
{
	return camUp;
}

void Camera::setCamPos(float x, float y, float z, float w)
{
	camPos = XMVectorSet(x, y, z, w);
}
