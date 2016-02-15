#include<iostream>
#include<vector>
#include<string>
#include<iostream>


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

//test
#include"bth_image.h"

#include "Heightmap.h"

//Device and swap chain resources
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gContext = nullptr;
IDXGISwapChain* gSwapChain = nullptr;
ID3D11RenderTargetView* gRTV = nullptr;
ID3D11Texture2D* gDepthStencilBuffer = nullptr;
ID3D11DepthStencilView* gDepthStencilView = nullptr;
//Vertex shader resources
ID3D11Buffer* gBoxVertexBuffer = nullptr;
ID3D11InputLayout* gGeoVertexLayout = nullptr;
ID3D11VertexShader* gGeoVertexShader = nullptr;

ID3D11InputLayout* gLightVertexLayout = nullptr;
ID3D11VertexShader* gLightVertexShader = nullptr;
ID3D11Buffer* gLightVertexBuffer = nullptr;

ID3D11Buffer* gGSConstantBuffer = nullptr;
ID3D11Buffer* gGS2ConstantBuffer = nullptr;

//Fragment shader resources
ID3D11PixelShader* gGeoPixelShader = nullptr;
ID3D11PixelShader* gLightPixelShader = nullptr;
ID3D11Buffer* gGeoPSConstBuffer = nullptr;
ID3D11Buffer* gLightPSConstBuffer = nullptr;
ID3D11Buffer* gCamPSConstBuffer = nullptr;
ID3D11ShaderResourceView* gTextureView = nullptr;
ID3D11Resource* gTexture = nullptr;
ID3D11SamplerState* gSamplerState = nullptr;

//gBuffer resources
ID3D11ShaderResourceView* gGBufferSRV[4] = { nullptr };
ID3D11RenderTargetView* gGBufferRTV[4] = { nullptr };



//needed for heightmap
ID3D11Buffer* gVertexBufferheightmap = nullptr;
ID3D11Buffer* gIndexBufferheightmap = nullptr;
ID3D11ShaderResourceView *gTextureViewen = nullptr;

HWND gWinHandle = NULL;
int gWinWidth = 1200;
int gWinHeight = 950;

int gNumberOfVertices = 0;

int gSampleCountMain = 1;

Camera camera;

using namespace std;
using namespace DirectX;

Color Ka, Kd, Ks;
float Ns;
wstring textureFileName;

int numfaces;

void CreateShaders()
{

	//Compiling GeoVertex Shader and creating input layout
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(L"GeoVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gGeoVertexShader);

	D3D11_INPUT_ELEMENT_DESC geoInputDesc[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0},{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA, 0} };

	gDevice->CreateInputLayout(geoInputDesc, ARRAYSIZE(geoInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gGeoVertexLayout);
	
	pVS->Release();


	//Compiling LightVertex Shader and creating input layout
	D3DCompileFromFile(L"LightVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gLightVertexShader);

	D3D11_INPUT_ELEMENT_DESC lightInputDesc[] = {{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 }};

	gDevice->CreateInputLayout(lightInputDesc, ARRAYSIZE(lightInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gLightVertexLayout);

	pVS->Release();



	ID3DBlob* pPS = nullptr;
	HRESULT hr;

	ID3DBlob* errorBlob;
	hr = D3DCompileFromFile(L"LightFragment.hlsl", nullptr, nullptr, "PS_main", "ps_4_0", 0, 0, &pPS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gLightPixelShader);
	pPS->Release();


	hr = D3DCompileFromFile(L"GeoFragment.hlsl", nullptr, nullptr, "PS_main", "ps_4_0", 0, 0, &pPS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gGeoPixelShader);
	pPS->Release();
}

struct GS_CONSTANT_BUFFER
{
	GS_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(GS_CONSTANT_BUFFER));
	}
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 projViewWorldMatrix;
};

//Lightsource
struct PS_LIGHT_CONSTANT_BUFFER
{
	PS_LIGHT_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(PS_LIGHT_CONSTANT_BUFFER));
	}
	XMFLOAT3 dir;
	float pad;
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


struct PS_GEO_CONSTANT_BUFFER
{
	PS_GEO_CONSTANT_BUFFER()
	{
		ZeroMemory(this, sizeof(PS_GEO_CONSTANT_BUFFER));
	}
	//Light light;
	XMFLOAT4 Ka;
	XMFLOAT4 Kd;
	XMFLOAT4 Ks;
	//XMFLOAT3 cameraPos;
};


void CreateConstantBuffers()
{
	//Geometry Constant Buffer Creation
	XMMATRIX mWorld, mView, mProjection, projMatrix, viewMatrix;
	mWorld = XMMatrixRotationY(0.0f);
	mView = XMMatrixLookAtLH(camera.getCamPos(), camera.getCamPos() + camera.getCamForward(), camera.getCamUp());
	mProjection = XMMatrixPerspectiveLH(3.141592*0.45, gWinWidth / gWinHeight, 0.5f, 20.0f);
	projMatrix = mWorld * mView * mProjection;
	projMatrix = XMMatrixTranspose(projMatrix);
	viewMatrix = mWorld*mView;
	viewMatrix = XMMatrixTranspose(viewMatrix);
	mWorld = XMMatrixTranspose(mWorld);


	GS_CONSTANT_BUFFER gsConstData;
	XMStoreFloat4x4(&gsConstData.projViewWorldMatrix, projMatrix);
	XMStoreFloat4x4(&gsConstData.worldMatrix, mWorld);

	D3D11_BUFFER_DESC gsBufferDesc;
	memset(&gsBufferDesc, 0, sizeof(gsBufferDesc));
	gsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	gsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	gsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	gsBufferDesc.ByteWidth = sizeof(GS_CONSTANT_BUFFER);

	D3D11_SUBRESOURCE_DATA gsData;
	memset(&gsData, 0, sizeof(gsData));
	gsData.pSysMem = &gsConstData;

	gDevice->CreateBuffer(&gsBufferDesc, &gsData, &gGSConstantBuffer);

	//Pixel Constant Geometry Buffer Creation

	PS_GEO_CONSTANT_BUFFER psGeoConstData;
	//psConstData.light = light;
	psGeoConstData.Ka = XMFLOAT4(Ka.r, Ka.g, Ka.b, 1);
	psGeoConstData.Kd = XMFLOAT4(Kd.r, Kd.g, Kd.b, 1);
	psGeoConstData.Ks = XMFLOAT4(Ks.r, Ks.g, Ks.b, Ns);
	//psConstData.cameraPos = { 0.0,0.0f,3.0f };

	D3D11_BUFFER_DESC psGeoBufferDesc;
	ZeroMemory(&psGeoBufferDesc, sizeof(psGeoBufferDesc));
	psGeoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psGeoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	psGeoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psGeoBufferDesc.ByteWidth = sizeof(PS_GEO_CONSTANT_BUFFER);

	D3D11_SUBRESOURCE_DATA psGeoData;
	ZeroMemory(&psGeoData, sizeof(psGeoData));
	psGeoData.pSysMem = &psGeoConstData;
	
	HRESULT hr = gDevice->CreateBuffer(&psGeoBufferDesc, &psGeoData, &gGeoPSConstBuffer);

	if (FAILED(hr))
	{
		cout << "Fail GeoBuffer" << endl;
	}


	//Pixel Constant light buffer
	XMFLOAT3 lightPos = { 3.0f, 3.0f, 5.0f };

	PS_LIGHT_CONSTANT_BUFFER psLightConstData;
	psLightConstData.dir = lightPos;
	psLightConstData.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	psLightConstData.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	D3D11_BUFFER_DESC psLightBufferDesc;
	ZeroMemory(&psLightBufferDesc, sizeof(psLightBufferDesc));
	psLightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psLightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	psLightBufferDesc.ByteWidth = sizeof(PS_LIGHT_CONSTANT_BUFFER);

	D3D11_SUBRESOURCE_DATA psLightData;
	ZeroMemory(&psLightData, sizeof(psLightData));
	psLightData.pSysMem = &psLightConstData;

	 hr = gDevice->CreateBuffer(&psLightBufferDesc, &psLightData, &gLightPSConstBuffer);

	if (FAILED(hr))
	{
		cout << "Fail LightBuffer" << endl;
	}


	//Pixel Constant cam buffer
	PS_CAM_CONSTANT_BUFFER psCamConstData;
	XMFLOAT3 camPos;
	XMStoreFloat3(&camPos, camera.getCamPos());
	psCamConstData.camPos = camPos;

	
	D3D11_BUFFER_DESC psCamBufferDesc;
	ZeroMemory(&psCamBufferDesc, sizeof(psCamBufferDesc));
	psCamBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psCamBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	psCamBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psCamBufferDesc.ByteWidth = sizeof(PS_CAM_CONSTANT_BUFFER);


	D3D11_SUBRESOURCE_DATA psCamData;
	ZeroMemory(&psCamData, sizeof(psCamData));
	psCamData.pSysMem = &psCamConstData;


	hr = gDevice->CreateBuffer(&psCamBufferDesc, &psCamData, &gCamPSConstBuffer);
	if (hr == E_INVALIDARG)
	{
		cout << "Fail CamBuffer" << endl;
	}

	//Test Second gsConstBuffer
	mWorld = XMMatrixTranslation(3, 0, 0);
	projMatrix = mWorld * mView* mProjection;
	projMatrix = XMMatrixTranspose(projMatrix);
	mWorld = XMMatrixTranspose(mWorld);

	GS_CONSTANT_BUFFER gs2ConstData;
	XMStoreFloat4x4(&gs2ConstData.projViewWorldMatrix, projMatrix);
	XMStoreFloat4x4(&gs2ConstData.worldMatrix, mWorld);

	D3D11_SUBRESOURCE_DATA gs2Data;
	memset(&gs2Data, 0, sizeof(gs2Data));
	gs2Data.pSysMem = &gs2ConstData;

	gDevice->CreateBuffer(&gsBufferDesc, &gs2Data, &gGS2ConstantBuffer);
}



void UpdateFrame()
{
	XMFLOAT3 cameraPos;
	XMStoreFloat3(&cameraPos, camera.getCamPos());

	XMMATRIX mWorld, mView, mProjection, projMatrix, viewMatrix;
	mWorld = XMMatrixTranslation(0,0,0);
	mView = XMMatrixLookAtLH(camera.getCamPos(), camera.getCamPos() + camera.getCamForward(), camera.getCamUp());
	mProjection = XMMatrixPerspectiveLH(3.141592f*0.45f, ((float)gWinWidth) / ((float)gWinHeight), 0.5f, 2000.0f);
	projMatrix = mWorld * mView * mProjection;
	projMatrix = XMMatrixTranspose(projMatrix);
	viewMatrix = mWorld*mView;
	viewMatrix = XMMatrixTranspose(viewMatrix);
	mWorld = XMMatrixTranspose(mWorld);

	GS_CONSTANT_BUFFER gsConstData;
	XMStoreFloat4x4(&gsConstData.projViewWorldMatrix, projMatrix);
	XMStoreFloat4x4(&gsConstData.worldMatrix, mWorld);

	//Mapping, updating, unmapping GS_Shader
	D3D11_MAPPED_SUBRESOURCE mappedResourceGS;
	gContext->Map(gGSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceGS);
	GS_CONSTANT_BUFFER* GSDataPtr = (GS_CONSTANT_BUFFER*)mappedResourceGS.pData;
	*GSDataPtr = gsConstData;
	gContext->Unmap(gGSConstantBuffer, 0);

	//PS GEO shader
	PS_GEO_CONSTANT_BUFFER psGeoConstData;
	psGeoConstData.Ka = XMFLOAT4(Ka.r, Ka.g, Ka.b, 1);
	psGeoConstData.Kd = XMFLOAT4(Kd.r, Kd.g, Kd.b, 1);
	psGeoConstData.Ks = XMFLOAT4(Ks.r, Ks.g, Ks.b, Ns);
	

	//Mapping, updating, unmapping  PS_Shader
	D3D11_MAPPED_SUBRESOURCE mappedResourcePS;
	gContext->Map(gGeoPSConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourcePS);
	PS_GEO_CONSTANT_BUFFER* PSDataPtr = (PS_GEO_CONSTANT_BUFFER*)mappedResourcePS.pData;
	*PSDataPtr = psGeoConstData;
	gContext->Unmap(gGeoPSConstBuffer, 0);

	//PS Light shader cam update
//psConstData.cameraPos = cameraPos;

	PS_CAM_CONSTANT_BUFFER psCamConstData;
	psCamConstData.camPos = cameraPos;

	D3D11_MAPPED_SUBRESOURCE mappedCamResourcePS;
	gContext->Map(gCamPSConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCamResourcePS);
	PS_CAM_CONSTANT_BUFFER* PSCamDataPtr = (PS_CAM_CONSTANT_BUFFER*)mappedCamResourcePS.pData;
	*PSCamDataPtr = psCamConstData;
	gContext->Unmap(gCamPSConstBuffer, 0);




	//Test Second gsConstBuffer
	XMMATRIX m2World;
	m2World = XMMatrixTranslation(10, 0, 0);
	projMatrix = m2World * mView* mProjection;
	projMatrix = XMMatrixTranspose(projMatrix);
	m2World = XMMatrixTranspose(m2World);

	GS_CONSTANT_BUFFER gs2ConstData;
	XMStoreFloat4x4(&gs2ConstData.projViewWorldMatrix, projMatrix);
	XMStoreFloat4x4(&gs2ConstData.worldMatrix, m2World);

	D3D11_SUBRESOURCE_DATA gs2Data;
	memset(&gs2Data, 0, sizeof(gs2Data));
	gs2Data.pSysMem = &gs2ConstData;

	//Mapping, updating, unmapping GS_Shader
	D3D11_MAPPED_SUBRESOURCE mappedResourceGS2;
	gContext->Map(gGS2ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceGS2);
	GS_CONSTANT_BUFFER* GS2DataPtr = (GS_CONSTANT_BUFFER*)mappedResourceGS2.pData;
	*GS2DataPtr = gs2ConstData;
	gContext->Unmap(gGS2ConstantBuffer, 0);

}




void CreateTextureView()
{
	CreateWICTextureFromFile(gDevice, textureFileName.c_str(), &gTexture, &gTextureView);

	D3D11_SAMPLER_DESC sampDesc;
	memset(&sampDesc, 0, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	gDevice->CreateSamplerState(&sampDesc, &gSamplerState);
		

}

void CreateVertices()
{
	vector<Vertice> vertices;
	
	importObj("Crate2.obj", vertices, textureFileName, Ka, Kd, Ks, Ns);



	gNumberOfVertices = vertices.size();




	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertice) * vertices.size();
	
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = vertices.data();

	gDevice->CreateBuffer(&bufferDesc, &data, &gBoxVertexBuffer);
}

struct LightVertice
{
	float x, y, z;
};

void CreateLightVertice()
{
	float side = 1.0;

	LightVertice vertices[6] = {
		-side, -side, 0.0f,

		-side, side, 0.0f,

		side, -side, 0.0f,

		-side, side, 0.0f,

		side, side, 0.0f,

		side, -side, 0.0f
	};

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(vertices);

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = vertices;

	gDevice->CreateBuffer(&bufferDesc, &data, &gLightVertexBuffer);
}

void Render()
{
	// clear gBuffer to all 0's
	float clearColor[] = { 0, 0, 0, 0 };

	for (int i = 0; i < 4; i++)
	{
		gContext->ClearRenderTargetView(gGBufferRTV[i], clearColor);
	}
	//clear the depth buffer
	gContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	//Geometry Pass
	gContext->OMSetRenderTargets(4, gGBufferRTV, gDepthStencilView);


	gContext->PSSetShader(gGeoPixelShader, nullptr, 0);

//	UINT32 vertexSize = sizeof(float) * 5; // la till för verttexsizen
	UINT32 offset = 0;
	UINT32 geoVertexSize = sizeof(float) * 8;
	UINT32 geoOffset = 0;
	gContext->IASetVertexBuffers(0, 1, &gBoxVertexBuffer, &geoVertexSize, &geoOffset);
	gContext->VSSetConstantBuffers(0, 1, &gGSConstantBuffer);
	gContext->PSSetConstantBuffers(0, 1, &gGeoPSConstBuffer);
	gContext->PSSetShaderResources(0, 1, &gTextureView);
	gContext->PSSetSamplers(0, 1, &gSamplerState);
	gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gContext->IASetInputLayout(gGeoVertexLayout);

	gContext->VSSetShader(gGeoVertexShader, nullptr, 0);
	gContext->HSSetShader(nullptr, nullptr, 0);
	gContext->DSSetShader(nullptr, nullptr, 0);
	gContext->GSSetShader(nullptr, nullptr, 0);
	gContext->PSSetShader(gGeoPixelShader, nullptr, 0);

	gContext->Draw(gNumberOfVertices, 0);

	//Second Box
	gContext->VSSetConstantBuffers(0, 1, &gGS2ConstantBuffer);
	gContext->Draw(gNumberOfVertices, 0);


	//heightmapbuffers
	gContext->IASetVertexBuffers(0, 1, &gVertexBufferheightmap, &geoVertexSize, &offset);
	gContext->IASetIndexBuffer(gIndexBufferheightmap, DXGI_FORMAT_R32_UINT, 0);
	gContext->PSSetShaderResources(0, 1, &gTextureViewen);

	//heightmap
	gContext->DrawIndexed(numfaces * 3, 0, 0);


	//LightningPass
	UINT32 lightVertexSize = sizeof(float) * 3;
	UINT32 lightOffset = 0;


	gContext->OMSetRenderTargets(1, &gRTV, NULL);
	gContext->PSSetConstantBuffers(0, 1, &gCamPSConstBuffer);
	gContext->PSSetConstantBuffers(1, 1, &gLightPSConstBuffer);
	gContext->PSSetShaderResources(0, 4, gGBufferSRV); //SRV = shader resource view...
	gContext->IASetInputLayout(gLightVertexLayout);
	gContext->IASetVertexBuffers(0,1,&gLightVertexBuffer, &lightVertexSize, &lightOffset);
	gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	gContext->VSSetShader(gLightVertexShader, nullptr, 0);
	gContext->HSSetShader(nullptr, nullptr, 0);
	gContext->DSSetShader(nullptr, nullptr, 0);
	gContext->GSSetShader(nullptr, nullptr, 0);
	gContext->PSSetShader(gLightPixelShader, nullptr, 0);

	gContext->Draw(6,0);

}

void CreateGBuffer()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = gWinWidth;
	textureDesc.Height = gWinHeight;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.MipLevels = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = gSampleCountMain;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;
	
	ID3D11Texture2D* pTexture[4];

	//Creating the texture array
	for (int i = 0; i < 4; i++)
	{
		HRESULT hr = gDevice->CreateTexture2D(&textureDesc, NULL, &pTexture[i]);
		if (FAILED(hr))
		{
			cout << "Fail to create texture nr: " << i << endl;
		}
	}

	//Creating the gBufferRTV
	D3D11_RENDER_TARGET_VIEW_DESC gBufferRTVDesc;
	ZeroMemory(&gBufferRTVDesc, sizeof(gBufferRTVDesc));
	gBufferRTVDesc.Format = textureDesc.Format;
	gBufferRTVDesc.Texture2D.MipSlice = 0;
	gBufferRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < 4; i++)
	{
		HRESULT hr = gDevice->CreateRenderTargetView(pTexture[i], &gBufferRTVDesc, &gGBufferRTV[i]);
		if (FAILED(hr))
		{
			cout << "Fail to create RTV nr: " << i << endl;
		}
	}

	//Creating the gBufferSRV
	D3D11_SHADER_RESOURCE_VIEW_DESC gBufferSRVDesc;
	ZeroMemory(&gBufferSRVDesc, sizeof(gBufferSRVDesc));
	gBufferSRVDesc.Format = textureDesc.Format;
	gBufferSRVDesc.Texture2D.MostDetailedMip = 0;
	gBufferSRVDesc.Texture2D.MipLevels = 1;
	gBufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < 4; i++)
	{
		HRESULT hr = gDevice->CreateShaderResourceView(pTexture[i], &gBufferSRVDesc, &gGBufferSRV[i]);
		if (FAILED(hr))
		{
			cout << "Fail to create SRV nr: " << i << endl;
		}

	}

	for (int i = 0; i < 4; i++)
	{
		//Unbinding pTexture to the textures
		pTexture[i]->Release();
	}
}

int main()
{	
	InitDirect3D(gDevice,gContext, gSwapChain, gRTV, gDepthStencilBuffer, gDepthStencilView, gWinHandle, gWinWidth, gWinHeight);

	CreateShaders();

	CreateVertices();

	CreateLightVertice();

	CreateConstantBuffers();

	CreateTextureView();
	
	CreateGBuffer();

	numfaces=heigthmapcreater(gVertexBufferheightmap, gIndexBufferheightmap, gDevice, gTextureViewen);
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
			SHORT Space = GetAsyncKeyState(VK_SPACE);;
			SHORT LCtrl = GetAsyncKeyState(VK_LCONTROL);
			SHORT LShift = GetAsyncKeyState(VK_LSHIFT);
			
			camera.UpdateCamera(WKey, AKey, SKey, DKey, RKey, LMouse, Space, LCtrl, LShift);
			UpdateFrame();
			Render();
			gSwapChain->Present(0, 0);
		}
	}

	//Clean Up
	gDevice->Release();
	gContext->Release();
	gSwapChain->Release();
	gRTV->Release();
	
	gDepthStencilBuffer->Release();
	gDepthStencilView->Release();

	gBoxVertexBuffer->Release();
	gGeoVertexLayout->Release();
	gGeoVertexShader->Release();

	gLightVertexLayout->Release();
	gLightVertexShader->Release();

	gGSConstantBuffer->Release();
	gGS2ConstantBuffer->Release();

	gGeoPixelShader->Release();
	gLightPixelShader->Release();
	gGeoPSConstBuffer->Release();
	gLightPSConstBuffer->Release();
	gCamPSConstBuffer->Release();

	//heightmap
	gVertexBufferheightmap->Release();
	gIndexBufferheightmap->Release();

	gTextureView->Release();
	gTexture->Release();
	gSamplerState->Release();

	for (int i = 0; i < 4; i++)
	{
		gGBufferSRV[i]->Release();
	}

	for (int i = 0; i < 4; i++)
	{
		gGBufferRTV[i]->Release();
	}


	return 0;
}