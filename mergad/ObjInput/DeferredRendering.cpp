#include "DeferredRendering.h"
#include <iostream>
#include "WICTextureLoader.h"
#include "Structs.h"

using namespace DirectX;
using namespace std;

DeferredRendering::DeferredRendering(Camera &camera)
{
	InitDirect3D(Device, Context, SwapChain, RTV, DepthStencilBuffer, DepthStencilView, WinHandle, WinWidth, WinHeight);
	memset(&clearColor, 0, sizeof(clearColor));
	nrOfSamples = 1;
	CreateShaders();
	CreateDepthBufferShadowMap();
	CreateLightVertexBuffer();
	lightVertexSize = sizeof(float) * 3;
	lightOffset = 0;
	CreateBuffers(camera);
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->HSSetShader(nullptr, nullptr, 0);
	Context->DSSetShader(nullptr, nullptr, 0);
	Context->GSSetShader(nullptr, nullptr, 0);
}

void DeferredRendering::CreateShaders()
{
	ID3DBlob* pVS = nullptr;
	HRESULT hr;
	//Deferred rendering geo vertex shader
	D3DCompileFromFile(L"GeoVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	hr = Device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &GeoVertexShader);
	if (FAILED(hr))
		cout << "Failed to create Geometry Vertex shader" << endl;
	D3D11_INPUT_ELEMENT_DESC geoInputDesc[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA, 0 } };

	hr = Device->CreateInputLayout(geoInputDesc, ARRAYSIZE(geoInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &GeoVertexLayout);
	if (FAILED(hr))
		cout << "Failed to create Geometry input Layout" << endl;

	pVS->Release();

	//Deferred rendering and shadow map pixel shader
	D3DCompileFromFile(L"LightVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	hr = Device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &LightVertexShader);
	if (FAILED(hr))
		cout << "Failed to create vertex shader for lightpass" << endl;
	D3D11_INPUT_ELEMENT_DESC lightInputDesc[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 } };

	hr = Device->CreateInputLayout(lightInputDesc, ARRAYSIZE(lightInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &LightVertexLayout);
	if (FAILED(hr))
		cout << "Failed to create input layout for lightpass" << endl;
	pVS->Release();
	//Compiling LightShadowVertex Shader
	D3DCompileFromFile(L"LightShadowVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	hr = Device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &LightShadowVertexShader);
	if (FAILED(hr))
		cout << "Failed to create Vertex shader for shadowmap lightpass" << endl;
	D3D11_INPUT_ELEMENT_DESC LightShadowInputDesc[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA, 0 } };

	hr = Device->CreateInputLayout(LightShadowInputDesc, ARRAYSIZE(LightShadowInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &LightShadowVertexLayout);
	if (FAILED(hr))
		cout << "Failed to create the input layout for the Vertex shader used in the shadowmap lightpass" << endl;

	pVS->Release();

	ID3DBlob* pPS = nullptr;

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
	Device->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &LightPixelShader);
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
	Device->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &GeoPixelShader);
	pPS->Release();
}

void DeferredRendering::CreateBuffers(Camera &camera)
{
	//Geometry Constant Buffer Creation
	mView = XMMatrixLookAtLH(camera.getCamPos(), camera.getCamPos() + camera.getCamForward(), camera.getCamUp());
	mProjection = XMMatrixPerspectiveLH(3.141592f*0.45f, ((float)WinWidth) / ((float)WinHeight), 0.5f, 20.0f);
	projViewMatrix = mView * mProjection;
	projViewMatrix = XMMatrixTranspose(projViewMatrix);


	VS_CONSTANT_BUFFER gsConstData;
	XMStoreFloat4x4(&gsConstData.projViewWorldMatrix, projViewMatrix);


	D3D11_BUFFER_DESC gsBufferDesc;
	memset(&gsBufferDesc, 0, sizeof(gsBufferDesc));
	gsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	gsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	gsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	gsBufferDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);

	D3D11_SUBRESOURCE_DATA gsData;
	memset(&gsData, 0, sizeof(gsData));
	gsData.pSysMem = &gsConstData;

	HRESULT hr = Device->CreateBuffer(&gsBufferDesc, &gsData, &ProjViewBuffer);
	if (FAILED(hr))
		cout << "Failed to create geometry projectView constant buffer" << endl;
	//Pixel Constant Geometry Buffer Creation

	D3D11_BUFFER_DESC psGeoBufferDesc;
	ZeroMemory(&psGeoBufferDesc, sizeof(psGeoBufferDesc));
	psGeoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psGeoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	psGeoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psGeoBufferDesc.ByteWidth = sizeof(PS_CONSTANT_BUFFER);


	hr = Device->CreateBuffer(&psGeoBufferDesc,NULL, &GeoPSConstBuffer);
	if (FAILED(hr))
		cout << "Failed to create geometry pixelshader constant buffer" << endl;
	//Light pass pixelshader camera buffer

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


	hr = Device->CreateBuffer(&psCamBufferDesc, &psCamData, &CamPSConstBuffer);
	if (FAILED(hr))
		cout << "Failed to create pixelshader lightpass constant buffer containing camera position" << endl;

	//Pixel Constant light buffer
	XMVECTOR lightDir = { 1.0f, 1.0f, 1.0f };
	lightDir = XMVector3Normalize(lightDir);

	PS_LIGHT_CONSTANT_BUFFER psLightConstData;
	psLightConstData.dir = lightDir;
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

	hr = Device->CreateBuffer(&psLightBufferDesc, &psLightData, &LightPSConstBuffer);
	if (FAILED(hr))
		cout << "Failed to create pixelshader constant buffer containing light data" << endl;

	//light view proj buffer creation
	D3D11_BUFFER_DESC lightViewProjMatrix;
	ZeroMemory(&lightViewProjMatrix, sizeof(lightViewProjMatrix));
	lightViewProjMatrix.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightViewProjMatrix.Usage = D3D11_USAGE_DYNAMIC;
	lightViewProjMatrix.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightViewProjMatrix.ByteWidth = sizeof(LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER);

	mView = XMMatrixLookAtLH(XMVECTOR{ 3.0f, 3.0f, 3.0f }, XMVECTOR{ 0.0f, 0.0f, 0.0f }, XMVECTOR{ -1.0f, 1.0f, -1.0f });
	

	mProjection = XMMatrixOrthographicLH(8.0f, 8.0f, 0.5f, 20.0f);
	projViewMatrix = mView* mProjection;
	projViewMatrix = XMMatrixTranspose(projViewMatrix);

	LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER viewProjData;
	XMStoreFloat4x4(&viewProjData.viewProjMatrix, projViewMatrix);

	D3D11_SUBRESOURCE_DATA viewProjDataSub;
	memset(&viewProjDataSub, 0, sizeof(viewProjDataSub));
	viewProjDataSub.pSysMem = &viewProjData;

	hr = Device->CreateBuffer(&lightViewProjMatrix, &viewProjDataSub, &LightViewProjConstantBuffer);
	if (FAILED(hr))
		cout << "Failed to create light WVPmatrix constant buffer for usage in lightpass and shadowmapp pass" << endl;

	//World matrix buffer creation
	D3D11_BUFFER_DESC worldMatrix;
	ZeroMemory(&worldMatrix, sizeof(worldMatrix));
	worldMatrix.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	worldMatrix.Usage = D3D11_USAGE_DYNAMIC;
	worldMatrix.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	worldMatrix.ByteWidth = sizeof(WORLD_MATRIX_CONSTANT_BUFFER);

	hr = Device->CreateBuffer(&worldMatrix, NULL, &WorldBuffer);
	if (FAILED(hr))
		cout << "Failed to create world matrix constant buffer " << endl;

	////////////////////// G-buffer creation
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = WinWidth;
	textureDesc.Height = WinHeight;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.MipLevels = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = nrOfSamples;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture[4];

	//Creating the texture array
	for (int i = 0; i < 4; i++)
	{
		hr = Device->CreateTexture2D(&textureDesc, NULL, &pTexture[i]);
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
		hr = Device->CreateRenderTargetView(pTexture[i], &gBufferRTVDesc, &GBufferRTV[i]);
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
		hr = Device->CreateShaderResourceView(pTexture[i], &gBufferSRVDesc, &GBufferSRV[i]);
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

void DeferredRendering::CreateDepthBufferShadowMap()
{
	D3D11_TEXTURE2D_DESC descDepthShadow;
	ZeroMemory(&descDepthShadow, sizeof(D3D11_TEXTURE2D_DESC));
	descDepthShadow.Width = WinWidth;
	descDepthShadow.Height = WinHeight;
	descDepthShadow.MipLevels = 1;
	descDepthShadow.ArraySize = 1;
	descDepthShadow.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepthShadow.SampleDesc.Count = nrOfSamples;
	descDepthShadow.SampleDesc.Quality = 0;
	descDepthShadow.Usage = D3D11_USAGE_DEFAULT;
	descDepthShadow.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepthShadow.CPUAccessFlags = 0;
	descDepthShadow.MiscFlags = 0;

	HRESULT hr = Device->CreateTexture2D(&descDepthShadow, NULL, &ShadowDepthStencilBuffer);
	if (FAILED(hr))
		cout << "Failed to create texture for shadowmap depthstencil buffer" << endl;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = Device->CreateDepthStencilView(ShadowDepthStencilBuffer, &descDSV, &DepthStencilViewShadowMap);
	if (FAILED(hr))
		cout << "Failed to create shadowmap depthstencil buffer" << endl;

	// Create the shader-resource view from the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	hr = Device->CreateShaderResourceView(ShadowDepthStencilBuffer, &srDesc, &SRVShadowMap);
	if (FAILED(hr))
		cout << "Failed to create shadowmap depthstencil resource view" << endl;
}

void DeferredRendering::CreateLightVertexBuffer()
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

	HRESULT hr = Device->CreateBuffer(&bufferDesc, &data, &LightVertexBuffer);
	if (FAILED(hr))
		cout << "Failed to create light vertex buffer" << endl;
}

void DeferredRendering::SetGeoPass()
{
	for (int i = 0; i < 4; i++)
	{
		Context->ClearRenderTargetView(GBufferRTV[i], clearColor);
	}
	Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	Context->OMSetRenderTargets(4, GBufferRTV, DepthStencilView);

	Context->VSSetConstantBuffers(0, 1, &ProjViewBuffer);
	Context->VSSetConstantBuffers(1, 1, &WorldBuffer);
	Context->PSSetConstantBuffers(0, 1, &GeoPSConstBuffer);
	Context->IASetInputLayout(GeoVertexLayout);

	Context->VSSetShader(GeoVertexShader, nullptr, 0);
	Context->PSSetShader(GeoPixelShader, nullptr, 0);
}

void DeferredRendering::SetLightPass()
{
	Context->OMSetRenderTargets(1, &RTV, NULL);
	Context->PSSetConstantBuffers(0, 1, &CamPSConstBuffer);
	Context->PSSetConstantBuffers(1, 1, &LightPSConstBuffer);
	Context->PSSetConstantBuffers(2, 1, &LightViewProjConstantBuffer);
	Context->PSSetShaderResources(0, 4, GBufferSRV);
	Context->PSSetShaderResources(4, 1, &SRVShadowMap);
	Context->IASetInputLayout(LightVertexLayout);
	Context->IASetVertexBuffers(0, 1, &LightVertexBuffer, &lightVertexSize, &lightOffset);

	Context->VSSetShader(LightVertexShader, nullptr, 0);
	Context->PSSetShader(LightPixelShader, nullptr, 0);

	Context->Draw(6, 0);
}

void DeferredRendering::SetShadowMapPass()
{
	Context->ClearDepthStencilView(DepthStencilViewShadowMap, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

	Context->OMSetRenderTargets(0, NULL, DepthStencilViewShadowMap);
	Context->VSSetConstantBuffers(0, 1, &LightViewProjConstantBuffer);
	Context->VSSetConstantBuffers(1, 1, &WorldBuffer);
	Context->PSSetConstantBuffers(0, 0, NULL);
	Context->PSSetShaderResources(0, 0, NULL);
	Context->PSSetSamplers(0, 0, NULL);
	Context->IASetInputLayout(LightShadowVertexLayout);

	Context->VSSetShader(LightShadowVertexShader, nullptr, 0);
	Context->PSSetShader(nullptr, nullptr, 0);
}

void DeferredRendering::UpdateFrame(Camera &camera)
{
	XMFLOAT3 cameraPos;
	XMStoreFloat3(&cameraPos, camera.getCamPos());

	mView = XMMatrixLookAtLH(camera.getCamPos(), camera.getCamPos() + camera.getCamForward(), camera.getCamUp());
	mProjection = XMMatrixPerspectiveLH(3.141592f*0.45f, ((float)WinWidth) / ((float)WinHeight), 0.5f, 2000.0f);
	projViewMatrix = mView * mProjection;
	projViewMatrix = XMMatrixTranspose(projViewMatrix);


	VS_CONSTANT_BUFFER gsConstData;
	XMStoreFloat4x4(&gsConstData.projViewWorldMatrix, projViewMatrix);

	//Mapping, updating, unmapping VS_Shader
	D3D11_MAPPED_SUBRESOURCE mappedResourceGS;
	Context->Map(ProjViewBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceGS);
	VS_CONSTANT_BUFFER* GSDataPtr = (VS_CONSTANT_BUFFER*)mappedResourceGS.pData;
	*GSDataPtr = gsConstData;
	Context->Unmap(ProjViewBuffer, 0);


	//PS Light shader cam update
	PS_CAM_CONSTANT_BUFFER psCamConstData;
	psCamConstData.camPos = cameraPos;

	D3D11_MAPPED_SUBRESOURCE mappedCamResourcePS;
	Context->Map(CamPSConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCamResourcePS);
	PS_CAM_CONSTANT_BUFFER* PSCamDataPtr = (PS_CAM_CONSTANT_BUFFER*)mappedCamResourcePS.pData;
	*PSCamDataPtr = psCamConstData;
	Context->Unmap(CamPSConstBuffer, 0);
}

DeferredRendering::~DeferredRendering()
{
	ShadowDepthStencilBuffer->Release();
	SRVShadowMap->Release();
	LightVertexBuffer->Release();
	GeoVertexShader->Release();
	GeoVertexLayout->Release();
	GeoPixelShader->Release();
	ProjViewBuffer->Release();
	GeoPSConstBuffer->Release();
	DepthStencilViewShadowMap->Release();
	WorldBuffer->Release();
	LightShadowVertexShader->Release();
	LightShadowVertexLayout->Release();
	LightVertexShader->Release();
	LightVertexLayout->Release();
	LightPixelShader->Release();
	CamPSConstBuffer->Release();
	LightPSConstBuffer->Release();
	LightViewProjConstantBuffer->Release();
	for (int i = 0; i < 4; i++)
		GBufferSRV[i]->Release();
	for (int i = 0; i < 4; i++)
		GBufferRTV[i]->Release();
	Device->Release();
	Context->Release();
	SwapChain->Release();
	RTV->Release();
	DepthStencilBuffer->Release();
	DepthStencilView->Release();
}

int DeferredRendering::getWinHeight()
{
	return WinHeight;
}

int DeferredRendering::getWinWidth()
{
	return WinWidth;
}

XMMATRIX DeferredRendering::getProjectionMatrix()
{
	return mProjection;
}

XMMATRIX DeferredRendering::getViewMatrix()
{
	return mView;
}
