#include "QuadTreeNode.h"

#include<iostream>

using namespace std;


QuadTreeNode::QuadTreeNode()
{
	ZeroMemory(this, sizeof(QuadTreeNode));
}

QuadTreeNode::~QuadTreeNode()
{
	for (int i = 0; i < 4; i++)
	{
		if (!(childs[i] == nullptr))
			delete childs[i];
	}

}

int QuadTreeNode::BBPlaneTest(XMFLOAT3 BBMin, XMFLOAT3 BBMax, Plane plane)
{
	XMFLOAT3 c;
	c.x = (BBMax.x + BBMin.x) / 2;
	c.y = (BBMax.y + BBMin.y) / 2;
	c.z = (BBMax.z + BBMin.z) / 2;

	XMFLOAT3 h;
	h.x = (BBMax.x - BBMin.x) / 2;
	h.y = (BBMax.y - BBMin.y) / 2;
	h.z = (BBMax.z - BBMin.z) / 2;

	float e = h.x*abs(plane.normal.x) + h.y*abs(plane.normal.y) + h.z*abs(plane.normal.z);

	float s = c.x*plane.normal.x + c.y*plane.normal.y + c.z*plane.normal.z + plane.distance;

	if (s - e > 0)
	{
		return 0; //Outside
	}
	else if (s + e < 0)
	{
		return 1; //Inside
	}
	return 2; //Intersecting
}

void QuadTreeNode::CreateNodes(QuadTreeNode &node, vector<objectData> &data, int depth)
{
	if (depth > 4)
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

		Plane plane1; // Z-plane
		plane1.distance = -(node.BBMin.x + tmp.x / 2);
		plane1.normal = { 1,0,0 };
		int boxStatus1; // Outside = 0, Inside = 1, Intersecting = 2

		Plane plane2; // X-plane
		plane2.distance = -(node.BBMin.z + tmp.z / 2);
		plane2.normal = { 0,0,1 };
		int boxStatus2;



		for (int i = 0; i < data.size(); i++)
		{
			boxStatus1 = BBPlaneTest(data[i].BBMin, data[i].BBMax, plane1);

			boxStatus2 = BBPlaneTest(data[i].BBMin, data[i].BBMax, plane2);

			if (boxStatus1 == 0)//Right half
			{
				if (boxStatus2 == 0) //Upper
				{
					childList[2].push_back(data[i]);
				}
				else if (boxStatus2 == 1) //Lower
				{
					childList[3].push_back(data[i]);
				}
				else//Intersecting
				{
					childList[2].push_back(data[i]);
					childList[3].push_back(data[i]);
				}
			}
			else if (boxStatus1 == 1)//Left half
			{
				if (boxStatus2 == 0)//Upper
				{
					childList[1].push_back(data[i]);
				}
				else if (boxStatus2 == 1)//Lower
				{
					childList[0].push_back(data[i]);
				}
				else//Intersecting
				{
					childList[0].push_back(data[i]);
					childList[1].push_back(data[i]);
				}
			}
			else//Intersecting
			{
				if (boxStatus2 == 0)//Upper
				{
					childList[1].push_back(data[i]);
					childList[2].push_back(data[i]);
				}
				else if (boxStatus2 == 1)//Lower
				{
					childList[0].push_back(data[i]);
					childList[3].push_back(data[i]);
				}
				else//Intersecting
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

void QuadTreeNode::CreateQuadTree(vector<objectData> &data, int depth)
{
	CreateNodes(*this, data, depth);
}

void QuadTreeNode::TraverseTree(QuadTreeNode &node, vector<bool> &drawList, Plane* planes)
{
	if (node.childs[0] == nullptr)
	{
		for (int i = 0; i < node.indiceData.size(); i++)
		{
			drawList[node.indiceData[i]] = true;
		}
	}
	else
	{
		int testResult; //BB against projViewPlanes test result
		bool finalResult = true;
		for (int i = 0; i < 6; i++)
		{
			testResult = BBPlaneTest(node.BBMin, node.BBMax, planes[i]);
			if (!testResult)
			{
				finalResult = false;
				break;
			}
		}
		if (finalResult)
		{
			for (int i = 0; i < 4; i++)
			{
				TraverseTree(*node.childs[i], drawList, planes);
			}
		}

	}
}

float length(XMFLOAT3 v)
{
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

void QuadTreeNode::QuadTreeTest(XMFLOAT4X4 M, vector<bool> &drawList)
{
	Plane planes[6];

	//Left
	planes[0].normal.x = -(M._41 + M._11);
	planes[0].normal.y = -(M._42 + M._12);
	planes[0].normal.z = -(M._43 + M._13);
	planes[0].distance = -(M._44 + M._14);

	//Right
	planes[1].normal.x = -(M._41 - M._11);
	planes[1].normal.y = -(M._42 - M._12);
	planes[1].normal.z = -(M._43 - M._13);
	planes[1].distance = -(M._44 - M._14);

	//Top
	planes[2].normal.x = -(M._41 - M._21);
	planes[2].normal.y = -(M._42 - M._22);
	planes[2].normal.z = -(M._43 - M._23);
	planes[2].distance = -(M._44 - M._24);

	//Bottom
	planes[3].normal.x = -(M._41 + M._21);
	planes[3].normal.y = -(M._42 + M._22);
	planes[3].normal.z = -(M._43 + M._23);
	planes[3].distance = -(M._44 + M._24);

	//Near
	planes[4].normal.x = -(M._41 + M._31);
	planes[4].normal.y = -(M._42 + M._32);
	planes[4].normal.z = -(M._43 + M._33);
	planes[4].distance = -(M._44 + M._34);

	//Far
	planes[5].normal.x = -(M._41 - M._31);
	planes[5].normal.y = -(M._42 - M._32);
	planes[5].normal.z = -(M._43 - M._33);
	planes[5].distance = -(M._44 - M._34);

	//Normalize
	for (int i = 0; i < 6; i++)
	{
		float denom = 1.0f / length(planes[i].normal);
		planes[i].normal.x *= denom;
		planes[i].normal.y *= denom;
		planes[i].normal.z *= denom;
		planes[i].distance *= denom;
	}

	//Resetting the drawList
	for (int i = 0; i < drawList.size(); i++)
	{
		drawList[i] = false;
	}

	TraverseTree(*this, drawList, planes);


}

