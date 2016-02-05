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




ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gContext = nullptr;
IDXGISwapChain* gSwapChain = nullptr;
ID3D11RenderTargetView* gRTV = nullptr;

ID3D11Texture2D* gDepthStencilBuffer = nullptr;
ID3D11DepthStencilView* gDepthStencilView = nullptr;

ID3D11Buffer* gVertexBuffer = nullptr;
ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;

ID3D11Buffer* gGSConstantBuffer = nullptr;
ID3D11Buffer* gGS2ConstantBuffer = nullptr;

ID3D11PixelShader* gPixelShader = nullptr;
ID3D11Buffer* gPSConstantBuffer = nullptr;
ID3D11ShaderResourceView* gTextureView = nullptr;
ID3D11Resource* gTexture = nullptr;
ID3D11SamplerState* gSamplerState = nullptr;



HWND gWinHandle = NULL;
int gWinWidth = 1200;
int gWinHeight = 950;

int gNumberOfVertices = 0;

Camera camera;

using namespace std;
using namespace DirectX;

Color Ka, Kd, Ks;
float Ns;
wstring textureFileName;

void CreateShaders()
{
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(L"Vertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0},{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA, 0} };

	gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
	
	pVS->Release();

	ID3DBlob* pPS = nullptr;
	HRESULT hr;

	ID3DBlob* errorBlob;
	hr = D3DCompileFromFile(L"Fragment.hlsl", nullptr, nullptr, "PS_main", "ps_4_0", 0, 0, &pPS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);

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
struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}
	XMFLOAT3 dir;
	float pad;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

struct PS_CONSTANT_BUFFER
{
	Light light;
	XMFLOAT4 Ka;
	XMFLOAT4 Kd;
	XMFLOAT4 Ks;
	float Ns;
	XMFLOAT3 cameraPos;
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

	//Pixel Constant Buffer Creation
	XMFLOAT3 lightPos = { 0.0f, 0.0f, 5.0f };

	Light light;
	light.dir = lightPos;
	light.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);


	PS_CONSTANT_BUFFER psConstData;
	psConstData.light = light;
	psConstData.Ka = XMFLOAT4(Ka.r, Ka.g, Ka.b, 1);
	psConstData.Kd = XMFLOAT4(Kd.r, Kd.g, Kd.b, 1);
	psConstData.Ks = XMFLOAT4(Ks.r, Ks.g, Ks.b, 1);
	psConstData.Ns = Ns;
	psConstData.cameraPos = { 0.0,0.0f,3.0f };

	D3D11_BUFFER_DESC psBufferDesc;
	ZeroMemory(&psBufferDesc, sizeof(psBufferDesc));
	psBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	psBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psBufferDesc.ByteWidth = sizeof(PS_CONSTANT_BUFFER);

	D3D11_SUBRESOURCE_DATA psData;
	ZeroMemory(&psData, sizeof(psData));
	psData.pSysMem = &psConstData;

	gDevice->CreateBuffer(&psBufferDesc, &psData, &gPSConstantBuffer);


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

//test
void CreateTextureViewTest()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));
	textureDesc.Height = BTH_IMAGE_HEIGHT;
	textureDesc.Width = BTH_IMAGE_WIDTH;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MiscFlags = 0;
	textureDesc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = nullptr;
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(data));
	data.pSysMem = (void*)BTH_IMAGE_DATA;
	data.SysMemPitch = sizeof(char) * 4 * BTH_IMAGE_WIDTH;
	HRESULT hr;

	ID3DBlob* errorBlob;

	hr = gDevice->CreateTexture2D(&textureDesc, &data, &pTexture);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	memset(&resourceViewDesc, 0, sizeof(resourceViewDesc));
	resourceViewDesc.Format = textureDesc.Format;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;


	hr = gDevice->CreateShaderResourceView(pTexture, &resourceViewDesc, &gTextureView);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	pTexture->Release();

	D3D11_SAMPLER_DESC sampDesc;
	memset(&sampDesc, 0, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MAXIMUM_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	gDevice->CreateSamplerState(&sampDesc, &gSamplerState);

}

//test
int rotationIncrement = 0;
void Update()
{
	XMFLOAT3 cameraPos;
	XMStoreFloat3(&cameraPos, camera.getCamPos());

	XMMATRIX mWorld, mView, mProjection, projMatrix, viewMatrix;
	mWorld = XMMatrixTranslation(0,0,0);
	mView = XMMatrixLookAtLH(camera.getCamPos(), camera.getCamPos() + camera.getCamForward(), camera.getCamUp());
	mProjection = XMMatrixPerspectiveLH(3.141592f*0.45f, ((float)gWinWidth) / ((float)gWinHeight), 0.5f, 20.0f);
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

	//PS shader
	XMFLOAT3 lightPos = { 3.0f, 3.0f, 5.0f };
	
	Light light;
	light.dir = lightPos;
	light.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);


	PS_CONSTANT_BUFFER psConstData;
	psConstData.light = light;
	psConstData.Ka = XMFLOAT4(Ka.r, Ka.g, Ka.b, 1);
	psConstData.Kd = XMFLOAT4(Kd.r, Kd.g, Kd.b, 1);
	psConstData.Ks = XMFLOAT4(Ks.r, Ks.g, Ks.b, 1);
	psConstData.Ns = Ns;
	psConstData.cameraPos = cameraPos;

	//Mapping, updating, unmapping  PS_Shader
	D3D11_MAPPED_SUBRESOURCE mappedResourcePS;
	gContext->Map(gPSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourcePS);
	PS_CONSTANT_BUFFER* PSDataPtr = (PS_CONSTANT_BUFFER*)mappedResourcePS.pData;
	*PSDataPtr = psConstData;
	gContext->Unmap(gPSConstantBuffer, 0);

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

	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);
}


void Render()
{
	// clear the back buffer to a deep blue
	float clearColor[] = { 0, 0, 0, 1 };
	gContext->ClearRenderTargetView(gRTV, clearColor);
	//clear the depth buffer
	gContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);


	gContext->VSSetShader(gVertexShader, nullptr, 0);
	gContext->HSSetShader(nullptr, nullptr, 0);
	gContext->DSSetShader(nullptr, nullptr, 0);
	gContext->GSSetShader(nullptr, nullptr, 0);
	gContext->PSSetShader(gPixelShader, nullptr, 0);


	UINT32 vertexSize = sizeof(float) * 8;
	UINT32 offset = 0;
	gContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gContext->VSSetConstantBuffers(0, 1, &gGSConstantBuffer);
	gContext->PSSetConstantBuffers(0, 1, &gPSConstantBuffer);
	gContext->PSSetShaderResources(0, 1, &gTextureView);
	gContext->PSSetSamplers(0, 1, &gSamplerState);
	gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gContext->IASetInputLayout(gVertexLayout);

	//gContext->Draw(36, 0);
	gContext->Draw(gNumberOfVertices, 0);

	//Second Box
	gContext->VSSetConstantBuffers(0, 1, &gGS2ConstantBuffer);
	gContext->Draw(gNumberOfVertices, 0);
}

int main()
{	
	InitDirect3D(gDevice,gContext, gSwapChain, gRTV, gDepthStencilBuffer, gDepthStencilView, gWinHandle, gWinWidth, gWinHeight);

	CreateShaders();

	CreateVertices();

	CreateConstantBuffers();

	CreateTextureView();

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
			Update();
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

	gVertexBuffer->Release();
	gVertexLayout->Release();
	gVertexShader->Release();

	gGSConstantBuffer->Release();
	gGS2ConstantBuffer->Release();

	gPixelShader->Release();
	gPSConstantBuffer->Release();
	gTextureView->Release();
	gTexture->Release();
	gSamplerState->Release();

	return 0;
}