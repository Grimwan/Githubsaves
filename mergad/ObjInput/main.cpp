#include<vector>
#include<string>
#include<iostream>

#include "Heightmap.h"
Heightmap heightmap;

//Directx include
#include<Windows.h>
#include<d3d11.h>

#include<d3dcompiler.h>

#include"SimpleMath.h"
#include"SimpleMath.inl"

//Picture Reader
#include"WICTextureLoader.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

//Own includes
#include"importObj.h"
#include"createDirectx.h"
#include"Camera.h"
#include"DeferredRendering.h"
#include"Object.h"
#include"QuadTreeNode.h"
#include"Structs.h"

Camera camera;
Object boxes;
QuadTreeNode root;

bool mouseButtonPressedLastFrame = false;
int depth = 0;

using namespace std;
using namespace DirectX;

void Render(DeferredRendering &Deferred)
{
	Deferred.SetGeoPass();
	boxes.DrawGeo(Deferred.Context, Deferred.WorldBuffer, Deferred.GeoPSConstBuffer);

	heightmap.Draw(Deferred.Context, Deferred.WorldBuffer);
	Deferred.SetShadowMapPass();
	boxes.DrawShadowMap(Deferred.Context, Deferred.WorldBuffer);
	Deferred.SetLightPass();
}
void CreateBoxes(ID3D11Device* &device)
{
	boxes.LoadModel(device, "Crate2.obj");
	XMMATRIX mWorld;
	////First box
	//mWorld = XMMatrixTranslation(3, 7, -2);
	//mWorld = XMMatrixTranspose(mWorld);
	//boxes.Add(mWorld);
	////Second box
	//mWorld = XMMatrixTranslation(3, 7, 3);
	//mWorld = XMMatrixTranspose(mWorld);
	//boxes.Add(mWorld);

	int x = 20;
	int z = 20;

	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < x; j++)
		{
			mWorld = XMMatrixTranslation(10.0f * i - x*10.0f/2, 20.0f, 10.0f*j - z*10.0f / 2);
			mWorld = XMMatrixTranspose(mWorld);
			boxes.Add(mWorld);
		}
	}

}
void MousePicking(DeferredRendering &Deferred, Object &boxes)
{
	POINT Pos;
	GetCursorPos(&Pos); //mouse positions are in screen coordinates
	ScreenToClient(Deferred.WinHandle, &Pos); // mouse position are in client coordiantes
	float x = ((2.0f * Pos.x) / Deferred.getWinWidth() - 1.0f) / Deferred.getProjectionMatrix().r[0].m128_f32[0]; //Transforms both y and x to NDC
	float y = (1.0f - (2.0f * Pos.y) / Deferred.getWinHeight()) / Deferred.getProjectionMatrix().r[1].m128_f32[1]; //and then dividing to bring them into view space
	XMVECTOR ray = { x, y, 1.0f};
	XMVECTOR origin = { 0.0f, 0.0f, 0.0f};
	ray = XMVector3TransformNormal(ray, XMMatrixInverse(nullptr, Deferred.getViewMatrix())); //direction in Worldspace
	origin = XMVector3TransformCoord(origin, XMMatrixInverse(nullptr, Deferred.getViewMatrix())); //origin in worldspace
	float t = -1;
	int index = -1;
	for (int i = 0; i < boxes.GetSize(); i++)
	{
		XMMATRIX WorldMatrix = XMMatrixTranspose(XMLoadFloat4x4(&boxes.GetWorldMatrix(i)));
		XMVECTOR rayModel = XMVector3TransformNormal(ray, XMMatrixInverse(nullptr, WorldMatrix)); // direction in modelspace
		XMVECTOR rayOrigin = XMVector3TransformCoord(origin, XMMatrixInverse(nullptr, WorldMatrix)); // origin in modelspace
		rayModel = XMVector3Normalize(rayModel);
		bool hit = true;

		float tmin = -INFINITY;
		float tmax = INFINITY;
		XMVECTOR p = /*Bcenter*/ -rayOrigin; //Bcenter is (0, 0, 0) if local space
		XMVECTOR arr[] = { XMVECTOR {1.0f, 0.0f, 0.0f}, XMVECTOR{ 0.0f, 1.0f, 0.0f }, XMVECTOR{ 0.0f, 0.0f, 1.0f }};
		float arr1[] = { 1.0f, 1.0f, 1.0f };
		for (int i = 0; i < 3; i++)
		{
			XMVECTOR result = XMVector3Dot(arr[i], p);
			XMFLOAT3 tmp1;
			XMStoreFloat3(&tmp1, result);
			float e = tmp1.x;
			result = XMVector3Dot(arr[i], rayModel);
			XMFLOAT3 tmp2;
			XMStoreFloat3(&tmp2, result);
			float f = tmp2.x;
			if (abs(f) > 0.00001)
			{
				float t1 = (e + arr1[i]) / f;
				float t2 = (e - arr1[i]) / f;
				if (t1 > t2)
				{
					float temp = t1;
					t1 = t2;
					t2 = temp;
				}
				if (t1 > tmin)
					tmin = t1;
				if (t2 < tmax)
					tmax = t2;
				if (tmin > tmax)
				{
					hit = false;
					break;
				}
				if (tmax < 0)
				{
					hit = false;
					break;
				}
			}
			else if (-e - arr1[i] > 0 || -e + arr1[i] < 0)
			{
				hit = false;
				break;
			}
		}
		if (tmin > 0 && hit)
		{
			if (t == -1 || t > tmin)
			{
				t = tmin;
				index = i;
			}
		}
		else if(hit)
		{
			if (t == -1 || t > tmax)
			{
				t = tmax;
				index = i;
			}
		}
	}
	if(index != -1)
		boxes.ToggleLight(index);
	
	cout << index << endl;
}

void CreateQuadTree()
{
	vector<objectData> list;
	boxes.GetObjectData(list);
	root.BBMax = XMFLOAT3(heightmap.hminfo.terrainwidth/2, 255.0f, heightmap.hminfo.terrainheight/2);
	root.BBMin = XMFLOAT3(-(heightmap.hminfo.terrainwidth / 2), 0.0f, -(heightmap.hminfo.terrainheight / 2));
	root.CreateQuadTree(list, 1);
}



int main()
{

	DeferredRendering Deferred(camera);
	heightmap.create(Deferred);
	CreateBoxes(Deferred.Device);
	heightmap.Heightmapcreater("heightmap.bmp", Deferred.Device);
	CreateQuadTree();

	MSG windowMsg = { 0 };
	
	


	while (windowMsg.message != WM_QUIT)
	{
		if (PeekMessage(&windowMsg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&windowMsg);
			DispatchMessage(&windowMsg);
		}
		else
		{
			SHORT WKey = GetAsyncKeyState('W');
			SHORT AKey = GetAsyncKeyState('A');
			SHORT SKey = GetAsyncKeyState('S');
			SHORT DKey = GetAsyncKeyState('D');
			SHORT RKey = GetAsyncKeyState('R');
			SHORT LMouse = GetAsyncKeyState(VK_LBUTTON);
			SHORT RMouse = GetAsyncKeyState(VK_RBUTTON);
			SHORT Space = GetAsyncKeyState(VK_SPACE);
			SHORT LCtrl = GetAsyncKeyState(VK_LCONTROL);
			SHORT LShift = GetAsyncKeyState(VK_LSHIFT);


			camera.UpdateCamera(WKey, AKey, SKey, DKey, RKey, LMouse, Space, LCtrl, LShift);
			heightmap.hiting(camera);
			Deferred.UpdateFrame(camera);
			root.QuadTreeTest(Deferred.getProjViewMatrix(), boxes.drawList);
			if (RMouse)
			{
				mouseButtonPressedLastFrame = true;
			}
			else if (mouseButtonPressedLastFrame)
			{
				MousePicking(Deferred, boxes);
				mouseButtonPressedLastFrame = false;
			}
			XMFLOAT4 pos;
			DirectX::XMStoreFloat4(&pos, camera.getCamPos());
			cout << pos.x << " " << pos.z << endl;


			Render(Deferred);
			Deferred.SwapChain->Present(0, 0);
		}
	}
	return 0;
}