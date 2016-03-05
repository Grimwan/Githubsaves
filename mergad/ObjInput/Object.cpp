#include"Object.h"
#include"WICTextureLoader.h"
#include"Structs.h"




Object::Object()
{
	vertexBuffer = nullptr;
	textureView = nullptr;
	samplerState = nullptr;
	texture = nullptr;
}

Object::~Object()
{
	vertexBuffer->Release();
	texture->Release();
	textureView->Release();
	samplerState->Release();

}

void Object::LoadModel(ID3D11Device* &device, string fileName)
{
	vector<Vertice> verticeList;
	wstring textureFileName;
	Color tmpKa, tmpKd, tmpKs;
	float tmpNs;

	importObj(fileName, verticeList, textureFileName, tmpKa, tmpKd, tmpKs, tmpNs);

	Ka = XMFLOAT4(tmpKa.r, tmpKa.g, tmpKa.b, 1.0f);
	Kd = XMFLOAT4(tmpKd.r, tmpKd.g, tmpKd.b, 1.0f);
	Ks = XMFLOAT4(tmpKs.r, tmpKs.g, tmpKs.b, tmpNs);

	numberOfVertices = verticeList.size();

	//Geometry buffers creation
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertice) * numberOfVertices;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = verticeList.data();

	device->CreateBuffer(&bufferDesc, &data, &vertexBuffer);


	//TextureView and samplerstate creation

	CreateWICTextureFromFile(device, textureFileName.c_str(), &texture, &textureView);


	D3D11_SAMPLER_DESC sampDesc;
	memset(&sampDesc, 0, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampDesc, &samplerState);
}

void Object::SetVertexBuffer(ID3D11DeviceContext* &context)
{
	UINT32 vertexSize = sizeof(Vertice);
	UINT32 offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexSize, &offset);
}


void Object::UpdateWorldBuffer(ID3D11DeviceContext* &context, ID3D11Buffer* &buffer, int index)
{
	WORLD_MATRIX_CONSTANT_BUFFER data;
	data.worldMatrix = worldMatrices[index];
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WORLD_MATRIX_CONSTANT_BUFFER* DataPtr = (WORLD_MATRIX_CONSTANT_BUFFER*)mappedResource.pData;
	*DataPtr = data;
	context->Unmap(buffer, 0);
}

void Object::UpdateFragmentBuffer(ID3D11DeviceContext* &context, ID3D11Buffer* &buffer)
{
	PS_CONSTANT_BUFFER data;
	data.Ka = Ka;
	data.Kd = Kd;
	data.Ks = Ks;  //Contains Ns

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	PS_CONSTANT_BUFFER* dataPtr = (PS_CONSTANT_BUFFER*)mappedResource.pData;
	*dataPtr = data;
	context->Unmap(buffer, 0);

}

void Object::DrawGeo(ID3D11DeviceContext* &context, ID3D11Buffer* &worldBuffer, ID3D11Buffer* &fragmentBuffer)
{
	SetVertexBuffer(context);

	UpdateFragmentBuffer(context, fragmentBuffer);
	context->PSSetShaderResources(0, 1, &textureView);
	context->PSSetSamplers(0, 1, &samplerState);

	for (int i = 0; i < worldMatrices.size(); i++)
	{
		UpdateWorldBuffer(context, worldBuffer, i);
		context->Draw(numberOfVertices, 0);
	}

}

void Object::DrawShadowMap(ID3D11DeviceContext* &context, ID3D11Buffer* &worldBuffer)
{
	SetVertexBuffer(context);

	for (int i = 0; i < worldMatrices.size(); i++)
	{
		UpdateWorldBuffer(context, worldBuffer, i);
		context->Draw(numberOfVertices, 0);
	}

}

void Object::Add(XMFLOAT4X4 &matrix)
{
	worldMatrices.push_back(matrix);
}

void Object::Add(XMMATRIX &matrix)
{
	XMFLOAT4X4 tmpMatrix;
	XMStoreFloat4x4(&tmpMatrix, matrix);
	worldMatrices.push_back(tmpMatrix);
}

int Object::GetSize()
{
	return worldMatrices.size();
}

XMFLOAT4X4 Object::GetWorldMatrix(int position)
{
	return worldMatrices[position];
}
