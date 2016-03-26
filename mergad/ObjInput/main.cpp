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


void CreateNodes(QuadTreeNode &node, vector<objectData> &data, int depth)
{
	if (depth > 3)
	{
		for (int i = 0; i < data.size(); i++)
		{
			node.indiceData.push_back(data[i].index);
		}
		for (int i = 0; i < 4; i++)
		{
			node.childs[i] = nullptr;
		}
		return;
	}
	else
	{	
		vector<objectData> childList[4];

		XMFLOAT3 tmp;
		tmp.x = node.BBMax.x - node.BBMin.x;
		tmp.y = node.BBMax.y;
		tmp.z = node.BBMax.z - node.BBMin.z;

		node.childs[0] = new QuadTreeNode; // bottomleft
		node.childs[0]->BBMin = node.BBMin;
		node.childs[0]->BBMax.x = node.BBMin.x + tmp.x / 2;
		node.childs[0]->BBMax.y = node.BBMax.y;
		node.childs[0]->BBMax.z = node.BBMin.z + tmp.z / 2;

		node.childs[1] = new QuadTreeNode; // upperLeft
		node.childs[1]->BBMin = node.BBMin;
		node.childs[1]->BBMin.z = node.BBMin.z + tmp.z / 2;
		node.childs[1]->BBMax = node.BBMax;
		node.childs[1]->BBMax.x = node.BBMin.x + tmp.x / 2;

		node.childs[2] = new QuadTreeNode; // upperRight
		node.childs[2]->BBMin.x = node.BBMin.x + tmp.x / 2;
		node.childs[2]->BBMax.y = node.BBMax.y;
		node.childs[2]->BBMin.z = node.BBMin.z + tmp.z / 2;
		node.childs[2]->BBMax = node.BBMax;

		node.childs[3] = new QuadTreeNode; // bottomRight
		node.childs[3]->BBMin = node.BBMin;
		node.childs[3]->BBMin.x = node.BBMin.x + tmp.x / 2;
		node.childs[3]->BBMax = node.BBMax;
		node.childs[3]->BBMax.z = node.BBMin.z + tmp.z / 2;

		Plane plane1;
		plane1.distance = node.BBMin.x + tmp.x / 2;
		plane1.normal = { 1,0,0 };
		int boxStatus1; // Outside = 0, Inside = 1, Intersecting = 2

		Plane plane2;
		plane2.distance = node.BBMin.z + tmp.z / 2;
		plane2.normal = { 0,0,1 };
		int boxStatus2;

	

		for (int i = 0; i < data.size(); i++)
		{
			XMFLOAT3 c;
			c.x = (data[i].BBMax.x + data[i].BBMin.x) / 2;
			c.y = (data[i].BBMax.y + data[i].BBMin.y) / 2;
			c.z = (data[i].BBMax.z + data[i].BBMin.z) / 2;
			
			XMFLOAT3 h;
			h.x = (data[i].BBMax.x - data[i].BBMin.x) / 2;
			h.y = (data[i].BBMax.y - data[i].BBMin.y) / 2;
			h.z = (data[i].BBMax.z - data[i].BBMin.z) / 2;

			//Plane 1

			float e = h.x*plane1.normal.x + h.y*plane1.normal.y + h.z*plane1.normal.z;

			float s = c.x*plane1.normal.x + c.y*plane1.normal.y + c.z*plane1.normal.z + plane1.distance;

			if (s - e > 0)
			{
				boxStatus1 = 0; //Outside
			}
			else if (s + e < 0)
			{
				boxStatus1 = 1; //Inside
			}
			else
			{
				boxStatus1 = 2; //Intersecting
			}

			//Plane 2
 
			if (s - e > 0)
			{
				boxStatus2 = 0; //Outside
			}
			else if (s + e < 0)
			{
				boxStatus2 = 1; //Inside
			}
			else
			{
				boxStatus2 = 2; //Intersecting
			}

			if (boxStatus1 == 0)
			{
				if (boxStatus2 == 0)
				{
					childList[2].push_back(data[i]);
				}
				else if (boxStatus2 == 1)
				{
					childList[3].push_back(data[i]);
				}
				else
				{
					childList[2].push_back(data[i]);
					childList[3].push_back(data[i]);
				}
			}
			else if (boxStatus1 == 1)
			{
				if (boxStatus2 == 0)
				{
					childList[1].push_back(data[i]);
				}
				else if (boxStatus2 == 1)
				{
					childList[0].push_back(data[i]);
				}
				else
				{
					childList[0].push_back(data[i]);
					childList[1].push_back(data[i]);
				}
			}
			else
			{
				if (boxStatus2 == 0)
				{
					childList[1].push_back(data[i]);
					childList[2].push_back(data[i]);
				}
				else if (boxStatus2 == 1)
				{
					childList[0].push_back(data[i]);
					childList[3].push_back(data[i]);
				}
				else
				{
					childList[0].push_back(data[i]);
					childList[1].push_back(data[i]);
					childList[2].push_back(data[i]);
					childList[3].push_back(data[i]);
				}
			}
		

		}

		for (int i = 0; i < 4; i++)
			CreateNodes(*node.childs[i], childList[i], depth + 1);
	}
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
			
			Render(Deferred);
			Deferred.SwapChain->Present(0, 0);
		}
	}
	return 0;
}