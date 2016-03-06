#ifndef STRUCTS_H
#define STRUCTS_H

#include"SimpleMath.h"
#include"SimpleMath.inl"
using namespace DirectX;
struct VS_CONSTANT_BUFFER
{
	VS_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(VS_CONSTANT_BUFFER));
	}
	XMFLOAT4X4 projViewWorldMatrix;
};

//Lightsource
struct PS_LIGHT_CONSTANT_BUFFER
{
	PS_LIGHT_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(PS_LIGHT_CONSTANT_BUFFER));
	}
	XMVECTOR dir;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

struct PS_CAM_CONSTANT_BUFFER
{
	PS_CAM_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(PS_CAM_CONSTANT_BUFFER));
	}
	XMFLOAT3 camPos;
	float pad;
};


struct PS_CONSTANT_BUFFER
{
	PS_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(PS_CONSTANT_BUFFER));
	}
	XMFLOAT4 Ka;
	XMFLOAT4 Kd;
	XMFLOAT4 Ks;
};
struct WORLD_MATRIX_CONSTANT_BUFFER
{
	WORLD_MATRIX_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(WORLD_MATRIX_CONSTANT_BUFFER));
	}
	XMFLOAT4X4 worldMatrix;
};
struct LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER
{
	LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(WORLD_MATRIX_CONSTANT_BUFFER));
	}
	XMFLOAT4X4 viewProjMatrix;
};
struct LightVertice
{
	float x, y, z;
};
#endif