#ifndef QUADTREENODE_H
#define QUADTREENODE_H

#include "SimpleMath.h"
#include "SimpleMath.inl"
#include "Structs.h"
#include<vector>

using namespace DirectX;
using namespace std;

class QuadTreeNode
{
public:
	QuadTreeNode();
	~QuadTreeNode();
	XMFLOAT3 BBMin;
	XMFLOAT3 BBMax;
	std::vector<int> indiceData;
	QuadTreeNode* childs[4];
	void CreateQuadTree(vector<objectData> &data, int depth);
	void QuadTreeTest(XMFLOAT4X4 projViewMatrix, vector<bool> &drawList);
private:
	void CreateNodes(QuadTreeNode &node, vector<objectData> &data, int depth, int maxDepth);
	void TraverseTree(QuadTreeNode &node, vector<bool> &drawList, Plane* planes);
	int BBPlaneTest(XMFLOAT3 BBMin, XMFLOAT3 BBMax, Plane plane);
};
#endif