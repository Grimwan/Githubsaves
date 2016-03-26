#ifndef OBJECT_H
#define OBJECT_H

#include<d3d11.h>
#include<d3dcompiler.h>
#include<vector>
#include<string>

#include "SimpleMath.h"
#include "SimpleMath.inl"

#include"importObj.h"
#include"Structs.h"

using namespace std;
using namespace DirectX;

class Object
{
private:
	//Geometry variables
	ID3D11Buffer* vertexBuffer;
	vector<XMFLOAT4X4> worldMatrices;
	vector<XMFLOAT3> BBMax;
	vector<XMFLOAT3> BBMin;
	vector<bool> litUp;

	int numberOfVertices;
	void UpdateWorldBuffer(ID3D11DeviceContext* &context, ID3D11Buffer* &buffer, int index);
	void UpdateFragmentBuffer(ID3D11DeviceContext* &context, ID3D11Buffer* &buffer, bool litUp);
	

	//Color varibles
	ID3D11ShaderResourceView* textureView;
	ID3D11SamplerState* samplerState;
	ID3D11Resource* texture;
	XMFLOAT4 Ka;
	XMFLOAT4 Kd;
	XMFLOAT4 Ks;


public:
	vector<bool> drawList;

	Object();
	~Object();
	void LoadModel(ID3D11Device* &device, string fileName);
	void SetVertexBuffer(ID3D11DeviceContext* &context);
	void DrawGeo(ID3D11DeviceContext* &context, ID3D11Buffer* &worldBuffer, ID3D11Buffer* &fragmentBuffer);
	void DrawShadowMap(ID3D11DeviceContext* &context, ID3D11Buffer* &worldBuffer);

	void Add(XMFLOAT4X4 &matrix);
	void Add(XMMATRIX &matrix);
	int GetSize();
	XMFLOAT4X4 GetWorldMatrix(int position);
	void ToggleLight(int index);
	void GetObjectData(vector<objectData> &data);

};









#endif
