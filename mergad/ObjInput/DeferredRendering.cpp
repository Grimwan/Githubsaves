#include "DeferredRendering.h"
#include <iostream>
#include "WICTextureLoader.h"
#include "Structs.h"
#include <DXGI.h>

using namespace DirectX;
using namespace std;

DeferredRendering::DeferredRendering(Camera &camera)
{
	InitDirect3D(Device, Context, SwapChain, RTV, DepthStencilBuffer, DepthStencilView, backbufferUAV, WinHandle, WinWidth, WinHeight);
	memset(&clearColor, 0, sizeof(clearColor));
	nrOfSamples = 1;
	CreatePointLight();
	CreateRSStates();
	CreateShaders();
	CreatePointLightShaders();
	CreateComputeShader();
	CreateDepthBufferShadowMap();
	CreateLightVertexBuffer();
	CreatePointLightVertexBuffer();
	CreateBlendStates();
	CreatePLWorldBuffer();
	lightVertexSize = sizeof(float) * 3;
	lightOffset = 0;
	CreateBuffers(camera);
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->HSSetShader(nullptr, nullptr, 0);
	Context->DSSetShader(nullptr, nullptr, 0);
	Context->GSSetShader(nullptr, nullptr, 0);

	//Creating Screen Viewport
	ScreenViewPort.Height = WinHeight;
	ScreenViewPort.Width = WinWidth;
	ScreenViewPort.MaxDepth = 1.0f;
	ScreenViewPort.MinDepth = 0.0f;
	ScreenViewPort.TopLeftX = 0;
	ScreenViewPort.TopLeftY = 0;
}

void DeferredRendering::CreateShaders()
{	
	ID3DBlob* errorBlob;
	HRESULT hr;
	ID3DBlob* pVS = nullptr;

	//Deferred rendering geo vertex shader
	D3DCompileFromFile(L"GeoVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	Device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &GeoVertexShader);

	D3D11_INPUT_ELEMENT_DESC geoInputDesc[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA, 0 } };

	Device->CreateInputLayout(geoInputDesc, ARRAYSIZE(geoInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &GeoVertexLayout);

	pVS->Release();

	//Deferred rendering and shadow map pixel shader
	D3DCompileFromFile(L"LightVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	Device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &LightVertexShader);

	D3D11_INPUT_ELEMENT_DESC lightInputDesc[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 } };

	Device->CreateInputLayout(lightInputDesc, ARRAYSIZE(lightInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &LightVertexLayout);

	pVS->Release();
	//Compiling LightShadowVertex Shader
	D3DCompileFromFile(L"LightShadowVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, nullptr);
	Device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &LightShadowVertexShader);

	D3D11_INPUT_ELEMENT_DESC LightShadowInputDesc[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA, 0 } };

	Device->CreateInputLayout(LightShadowInputDesc, ARRAYSIZE(LightShadowInputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &LightShadowVertexLayout);


	pVS->Release();

	//Geometry shader
	ID3DBlob* pGS = nullptr;
	hr = D3DCompileFromFile(L"GeoGeometryShader.hlsl", nullptr, nullptr, "GS_main", "gs_4_0", 0, 0, &pGS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	Device->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &GeoGeometryShader);

	pGS->Release();


	//Pixel shader
	ID3DBlob* pPS = nullptr;



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
void DeferredRendering::CreatePointLightShaders()
{
	HRESULT hr;
	ID3DBlob* errorBlob;

	ID3DBlob* pVS = nullptr;
	hr = D3DCompileFromFile(L"PointLightVertex.hlsl", nullptr, nullptr, "VS_main", "vs_4_0", 0, 0, &pVS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	Device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &PointLightVertexShader);
	pVS->Release();

	ID3DBlob* pPS = nullptr;
	hr = D3DCompileFromFile(L"PointLightFragment.hlsl", nullptr, nullptr, "PS_main", "ps_4_0", 0, 0, &pPS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	Device->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &PointLightPixelShader);
	pPS->Release();
}

void DeferredRendering::CreatePointLightVertexBuffer()
{
	HRESULT hr;
	vector<Vertice> tmpList;
	wstring s1;
	Color c1, c2, c3;
	float f5;

	importObj("Crate2.obj", tmpList, s1, c1, c2, c3, f5);
	vector <LightVertice> verticeList;
	for (int i = 0; i < tmpList.size(); i++)
	{
		//Putting in a lightVertice list
		LightVertice tmp;
		tmp.x = tmpList[i].posX;
		tmp.y = tmpList[i].posY;
		tmp.z = tmpList[i].posZ;
		verticeList.push_back(tmp);

	}

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertice) * verticeList.size();

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = verticeList.data();

	hr = Device->CreateBuffer(&bufferDesc, &data, &LightBoxVertexBuffer);
	if (FAILED(hr))
	{
		cout << "Fail LightBoxVertexBuffer" << endl;
	}
}

void DeferredRendering::CreatePLWorldBuffer()
{
	D3D11_BUFFER_DESC worldMatrix;
	ZeroMemory(&worldMatrix, sizeof(worldMatrix));
	worldMatrix.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	worldMatrix.Usage = D3D11_USAGE_DYNAMIC;
	worldMatrix.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	worldMatrix.ByteWidth = sizeof(WORLD_MATRIX_CONSTANT_BUFFER);

	Device->CreateBuffer(&worldMatrix, NULL, &PLWorldBuffer);
}

void DeferredRendering::CreatePointLight()
{
	pointLight.cBufferData.pos = XMFLOAT3(8.0f, 3.0f, 15.0f);
	pointLight.cBufferData.range = 40.0f;
	pointLight.cBufferData.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pointLight.cBufferData.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMMATRIX tmpM;
	tmpM = XMMatrixScaling(pointLight.cBufferData.range, pointLight.cBufferData.range, pointLight.cBufferData.range);
	tmpM *= XMMatrixTranslation(pointLight.cBufferData.pos.x, pointLight.cBufferData.pos.y, pointLight.cBufferData.pos.z);
	tmpM = XMMatrixTranspose(tmpM);
	XMStoreFloat4x4(&pointLight.worldMatrix, tmpM);
}

void DeferredRendering::CreateRSStates()
{
	D3D11_RASTERIZER_DESC geoDesc;
	ZeroMemory(&geoDesc, sizeof(geoDesc));
	geoDesc.CullMode = D3D11_CULL_NONE;
	geoDesc.AntialiasedLineEnable = false;
	geoDesc.DepthBias = 0;
	geoDesc.DepthBiasClamp = 0.0f;
	geoDesc.DepthClipEnable = true;
	geoDesc.FillMode = D3D11_FILL_SOLID;
	geoDesc.FrontCounterClockwise = false;
	geoDesc.MultisampleEnable = false;
	geoDesc.ScissorEnable = false;
	geoDesc.SlopeScaledDepthBias = 0.0f;
	
	Device->CreateRasterizerState(&geoDesc, &GeoRSState);

	D3D11_RASTERIZER_DESC lightDesc;
	ZeroMemory(&lightDesc, sizeof(lightDesc));
	lightDesc.CullMode = D3D11_CULL_FRONT;
	lightDesc.AntialiasedLineEnable = false;
	lightDesc.DepthBias = 0;
	lightDesc.DepthBiasClamp = 0.0f;
	lightDesc.DepthClipEnable = true;
	lightDesc.FillMode = D3D11_FILL_SOLID;
	lightDesc.FrontCounterClockwise = false;
	lightDesc.MultisampleEnable = false;
	lightDesc.ScissorEnable = false;
	lightDesc.SlopeScaledDepthBias = 0.0f;

	Device->CreateRasterizerState(&lightDesc, &LightRSState);
}


void DeferredRendering::UpdateWorldBuffer(ID3D11Buffer *&worldBuffer, XMFLOAT4X4 worldMatrix)
{
	WORLD_MATRIX_CONSTANT_BUFFER worldConstData;
	worldConstData.worldMatrix = worldMatrix;

	D3D11_MAPPED_SUBRESOURCE mappedCamResourcePS;
	Context->Map(worldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCamResourcePS);
	WORLD_MATRIX_CONSTANT_BUFFER* worldDataPtr = (WORLD_MATRIX_CONSTANT_BUFFER*)mappedCamResourcePS.pData;
	*worldDataPtr = worldConstData;
	Context->Unmap(worldBuffer, 0);
}



void DeferredRendering::CreateComputeShader()
{
	ID3DBlob* errorBlob;
	HRESULT hr;
	ID3DBlob* pCS = nullptr;

	hr = D3DCompileFromFile(L"ComputeShaderFXAA.hlsl", nullptr, nullptr, "CS_main", "cs_5_0", 0, 0, &pCS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	hr = Device->CreateComputeShader(pCS->GetBufferPointer(), pCS->GetBufferSize(), nullptr, &FXAACS);
	if (FAILED(hr))
		cout << "Failed to create FXAACS" << endl;
	pCS->Release();
}

void DeferredRendering::setFXAA()
{
	Context->OMSetRenderTargets(0, NULL, NULL);
	Context->CSSetShaderResources(0, 1, &tmpSRV);
	Context->CSSetUnorderedAccessViews(0, 1, &backbufferUAV, NULL);
	Context->CSSetShader(FXAACS, nullptr, 0);
	Context->Dispatch(48, 38, 1);
}

void DeferredRendering::CreateBuffers(Camera &camera)
{
	//Geometry Constant Buffer Creation
	mView = XMMatrixLookAtLH(camera.getCamPos(), camera.getCamPos() + camera.getCamForward(), camera.getCamUp());
	mProjection = XMMatrixPerspectiveLH(3.141592f*0.45f, float(WinWidth) / float(WinHeight), 0.5f, 200.0f);
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

	Device->CreateBuffer(&gsBufferDesc, &gsData, &ProjViewBuffer);

	//Pixel Constant Geometry Buffer Creation



	D3D11_BUFFER_DESC psGeoBufferDesc;
	ZeroMemory(&psGeoBufferDesc, sizeof(psGeoBufferDesc));
	psGeoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psGeoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	psGeoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psGeoBufferDesc.ByteWidth = sizeof(PS_CONSTANT_BUFFER);


	Device->CreateBuffer(&psGeoBufferDesc,NULL, &GeoPSConstBuffer);

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


	HRESULT hr = Device->CreateBuffer(&psCamBufferDesc, &psCamData, &CamPSConstBuffer);
	if (hr == E_INVALIDARG)
	{
		cout << "Fail CamBuffer" << endl;
	}

	//Pixel Constant light buffer

	//Directional Light
	XMVECTOR lightDir = { 1.0f, 1.0f, 1.0f };
	lightDir = XMVector3Normalize(lightDir);
	PS_LIGHT_CONSTANT_BUFFER psLightConstData;
	XMStoreFloat3(&psLightConstData.dirLight.dir, lightDir);
	psLightConstData.dirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	psLightConstData.dirLight.diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	
	//QuadTreeTest
	//psLightConstData.dirLight.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	
	//Point Light
	psLightConstData.pointLight = pointLight.cBufferData;

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
	{
		cout << "Fail LightBuffer" << endl;
	}
	//light view proj buffer creation
	D3D11_BUFFER_DESC lightViewProjMatrix;
	ZeroMemory(&lightViewProjMatrix, sizeof(lightViewProjMatrix));
	lightViewProjMatrix.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightViewProjMatrix.Usage = D3D11_USAGE_DYNAMIC;
	lightViewProjMatrix.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightViewProjMatrix.ByteWidth = sizeof(LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER);

	shadowMapOrigin = XMVECTOR{ 200.0f, 200.0f, 200.0f };

	mLightView = XMMatrixLookAtLH(camera.getCamPos() + shadowMapOrigin, camera.getCamPos(), XMVECTOR{ -1.0f, 2.0f, -1.0f });
	mLightProjection = XMMatrixOrthographicLH(200.0f, 200.0f, 0.5f, 800.0f);
	lightProjViewMatrix = mLightView * mLightProjection;
	lightProjViewMatrix = XMMatrixTranspose(lightProjViewMatrix);

	LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER viewProjData;
	XMStoreFloat4x4(&viewProjData.viewProjMatrix, lightProjViewMatrix);

	D3D11_SUBRESOURCE_DATA viewProjDataSub;
	memset(&viewProjDataSub, 0, sizeof(viewProjDataSub));
	viewProjDataSub.pSysMem = &viewProjData;

	hr = Device->CreateBuffer(&lightViewProjMatrix, &viewProjDataSub, &LightViewProjConstantBuffer);
	if (FAILED(hr))
	{
		if (hr == E_INVALIDARG)
		{
			cout << "Fail gLightViewProjConstantBuffer" << endl;
		}
	}
	//World matrix buffer creation
	D3D11_BUFFER_DESC worldMatrix;
	ZeroMemory(&worldMatrix, sizeof(worldMatrix));
	worldMatrix.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	worldMatrix.Usage = D3D11_USAGE_DYNAMIC;
	worldMatrix.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	worldMatrix.ByteWidth = sizeof(WORLD_MATRIX_CONSTANT_BUFFER);

	hr = Device->CreateBuffer(&worldMatrix, NULL, &WorldBuffer);
	if (FAILED(hr))
	{
		if (hr == E_INVALIDARG)
		{
			cout << "Fail gWorldBuffer" << endl;
		}
	}
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
		HRESULT hr = Device->CreateTexture2D(&textureDesc, NULL, &pTexture[i]);
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
		HRESULT hr = Device->CreateRenderTargetView(pTexture[i], &gBufferRTVDesc, &GBufferRTV[i]);
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
		HRESULT hr = Device->CreateShaderResourceView(pTexture[i], &gBufferSRVDesc, &GBufferSRV[i]);
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
	//////////////////////computeShader resources creation
	D3D11_TEXTURE2D_DESC textDesc;
	ZeroMemory(&textDesc, sizeof(textDesc));
	textDesc.Width = WinWidth;
	textDesc.Height = WinHeight;
	textDesc.ArraySize = 1;
	textDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textDesc.MipLevels = 1;
	textDesc.Usage = D3D11_USAGE_DEFAULT;
	textDesc.SampleDesc.Count = nrOfSamples;
	textDesc.SampleDesc.Quality = 0;
	textDesc.CPUAccessFlags = 0;

	ID3D11Texture2D* FXAATexture;

	hr = Device->CreateTexture2D(&textDesc, NULL, &FXAATexture);
	if (FAILED(hr))
		cout << "Fail to create FXAA texture" << endl;

	//RTV
	D3D11_RENDER_TARGET_VIEW_DESC tmpRTVDesc;
	ZeroMemory(&tmpRTVDesc, sizeof(tmpRTVDesc));
	tmpRTVDesc.Format = textDesc.Format;
	tmpRTVDesc.Texture2D.MipSlice = 0;
	tmpRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = Device->CreateRenderTargetView(FXAATexture, &tmpRTVDesc, &tmpRTV);
	if (FAILED(hr))
		cout << "Fail to create tmpRTV " << endl;

	//SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC tmpSRVDesc;
	ZeroMemory(&tmpSRVDesc, sizeof(tmpSRVDesc));
	tmpSRVDesc.Format = textDesc.Format;
	tmpSRVDesc.Texture2D.MostDetailedMip = 0;
	tmpSRVDesc.Texture2D.MipLevels = 1;
	tmpSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	hr = Device->CreateShaderResourceView(FXAATexture, &tmpSRVDesc, &tmpSRV);
	if (FAILED(hr))
		cout << "Fail to create tmpSRV " << endl;

	FXAATexture->Release();
}

void DeferredRendering::CreateBlendStates()
{
	HRESULT hr;

	//Creation of Lightpass blendstate
	D3D11_BLEND_DESC lightBlendDesc;
	ZeroMemory(&lightBlendDesc, sizeof(lightBlendDesc));
	lightBlendDesc.AlphaToCoverageEnable = false;
	lightBlendDesc.IndependentBlendEnable = false;
	lightBlendDesc.RenderTarget[0].BlendEnable = true;
	lightBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	lightBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	lightBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	lightBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	lightBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	lightBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	lightBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = Device->CreateBlendState(&lightBlendDesc, &LightBlendState);
	if (FAILED(hr))
		cout << "Failed with LightBlendstate" << endl;

}

void DeferredRendering::CreateDepthBufferShadowMap()
{
	int size = 4096;

	ShadowMapViewPort.Height = size;
	ShadowMapViewPort.Width = size;
	ShadowMapViewPort.MaxDepth = 1.0f;
	ShadowMapViewPort.MinDepth = 0.0f;
	ShadowMapViewPort.TopLeftX = 0;
	ShadowMapViewPort.TopLeftY = 0;

	D3D11_TEXTURE2D_DESC descDepthShadow;
	ZeroMemory(&descDepthShadow, sizeof(D3D11_TEXTURE2D_DESC));
	descDepthShadow.Width = size;
	descDepthShadow.Height = size;
	descDepthShadow.MipLevels = 1;
	descDepthShadow.ArraySize = 1;
	descDepthShadow.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepthShadow.SampleDesc.Count = 1;
	descDepthShadow.SampleDesc.Quality = 0;
	descDepthShadow.Usage = D3D11_USAGE_DEFAULT;
	descDepthShadow.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepthShadow.CPUAccessFlags = 0;
	descDepthShadow.MiscFlags = 0;

	Device->CreateTexture2D(&descDepthShadow, NULL, &ShadowDepthStencilBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	Device->CreateDepthStencilView(ShadowDepthStencilBuffer, &descDSV, &DepthStencilViewShadowMap);

	// Create the shader-resource view from the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	ZeroMemory(&srDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	Device->CreateShaderResourceView(ShadowDepthStencilBuffer, &srDesc, &SRVShadowMap);

	//Create samplerState
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;

	Device->CreateSamplerState(&samplerDesc, &ShadowMapSamplerState);

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

	Device->CreateBuffer(&bufferDesc, &data, &LightVertexBuffer);
}



void DeferredRendering::SetGeoPass()
{
	for (int i = 0; i < 4; i++)
	{
		Context->ClearRenderTargetView(GBufferRTV[i], clearColor);
	}
	Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	Context->OMSetRenderTargets(4, GBufferRTV, DepthStencilView);
	Context->OMSetBlendState(NULL, NULL, 0xffffffff);
	Context->RSSetViewports(1, &ScreenViewPort);
	Context->RSSetState(GeoRSState);

	Context->VSSetConstantBuffers(0, 1, &WorldBuffer);
	Context->GSSetConstantBuffers(0, 1, &ProjViewBuffer);
	Context->GSSetConstantBuffers(1, 1, &CamPSConstBuffer);
	Context->PSSetConstantBuffers(0, 1, &GeoPSConstBuffer);
	Context->IASetInputLayout(GeoVertexLayout);

	Context->VSSetShader(GeoVertexShader, nullptr, 0);
	Context->GSSetShader(GeoGeometryShader, nullptr, 0);
	Context->PSSetShader(GeoPixelShader, nullptr, 0);
}

void DeferredRendering::SetLightPass()
{
	Context->OMSetRenderTargets(1, &tmpRTV, NULL); //FXAA
	Context->RSSetViewports(1, &ScreenViewPort);
	Context->ClearRenderTargetView(tmpRTV, clearColor);

	Context->IASetInputLayout(LightVertexLayout);
	Context->IASetVertexBuffers(0, 1, &LightVertexBuffer, &lightVertexSize, &lightOffset);
	Context->PSSetConstantBuffers(0, 1, &CamPSConstBuffer);
	Context->PSSetConstantBuffers(1, 1, &LightPSConstBuffer);
	Context->PSSetConstantBuffers(2, 1, &LightViewProjConstantBuffer);
	Context->PSSetShaderResources(0, 4, GBufferSRV);
	Context->PSSetShaderResources(4, 1, &SRVShadowMap);
	Context->PSSetSamplers(0, 1, &ShadowMapSamplerState);


	Context->VSSetShader(LightVertexShader, nullptr, 0);
	Context->GSSetShader(nullptr, nullptr, 0);
	Context->PSSetShader(LightPixelShader, nullptr, 0);

	//Dir light fullscreen quad
	Context->Draw(6, 0);

	//Setting up for point light
	Context->RSSetState(LightRSState);
	Context->OMSetBlendState(LightBlendState, NULL, 0xffffffff);

	Context->IASetVertexBuffers(0, 1, &LightBoxVertexBuffer, &lightVertexSize, &lightOffset);
	Context->VSSetConstantBuffers(0, 1, &PLWorldBuffer);
	Context->VSSetConstantBuffers(1, 1, &ProjViewBuffer);


	Context->VSSetShader(PointLightVertexShader, nullptr, 0);
	Context->GSSetShader(nullptr, nullptr, 0);
	Context->PSSetShader(PointLightPixelShader, nullptr, 0);


	//Point light box
	UpdateWorldBuffer(PLWorldBuffer, pointLight.worldMatrix);
	Context->Draw(36, 0);

}

void DeferredRendering::SetShadowMapPass()
{
	Context->ClearDepthStencilView(DepthStencilViewShadowMap, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	Context->OMSetRenderTargets(0, NULL, DepthStencilViewShadowMap);
	Context->RSSetViewports(1, &ShadowMapViewPort);

	Context->VSSetConstantBuffers(0, 1, &LightViewProjConstantBuffer);
	Context->VSSetConstantBuffers(1, 1, &WorldBuffer);
	Context->PSSetConstantBuffers(0, 0, NULL);
	Context->PSSetShaderResources(0, 0, NULL);
	Context->PSSetSamplers(0, 0, NULL);
	Context->IASetInputLayout(LightShadowVertexLayout);

	Context->VSSetShader(LightShadowVertexShader, nullptr, 0);
	Context->GSSetShader(nullptr, nullptr, 0);
	Context->PSSetShader(nullptr, nullptr, 0);
}

void DeferredRendering::UpdateFrame(Camera &camera)
{
	XMFLOAT3 cameraPos;
	XMStoreFloat3(&cameraPos, camera.getCamPos());
	cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << endl;
	XMMATRIX testProjViewMatrix, testViewMatrix, testProjMatrix;

	mView = XMMatrixLookAtLH(camera.getCamPos(), camera.getCamPos() + camera.getCamForward(), camera.getCamUp());
	projViewMatrix = mView * mProjection;
	projViewMatrix = XMMatrixTranspose(projViewMatrix);

	//Quadtree test
	//testViewMatrix = XMMatrixLookAtLH(XMVECTOR{ cameraPos.x, 100.0f, cameraPos.z ,1.0f }, XMVECTOR{ cameraPos.x, 0.0f, cameraPos.z, 1.0f }, XMVECTOR{ 0.0f, 0.0f, -1.0f, 1.0f });
	//testProjMatrix = XMMatrixOrthographicLH(300.0f, 300.0f, 0.5f, 150.0f);
	//testProjViewMatrix = testViewMatrix * testProjMatrix;
	//testProjViewMatrix = XMMatrixTranspose(testProjViewMatrix);

	VS_CONSTANT_BUFFER gsConstData;
	XMStoreFloat4x4(&gsConstData.projViewWorldMatrix, projViewMatrix);

	//Quadtree test
	//XMStoreFloat4x4(&gsConstData.projViewWorldMatrix, testProjViewMatrix);


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

	//Light ProjViewBuffer update
	mLightView = XMMatrixLookAtLH(camera.getCamPos() + shadowMapOrigin, camera.getCamPos(), XMVector3Normalize(XMVECTOR{ -1.0f, 2.0f, -1.0f }));
	lightProjViewMatrix = mLightView * mLightProjection;
	lightProjViewMatrix = XMMatrixTranspose(lightProjViewMatrix);

	LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER lightProjConstData;
	XMStoreFloat4x4(&lightProjConstData.viewProjMatrix, lightProjViewMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedLightProjResource;
	Context->Map(LightViewProjConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedLightProjResource);
	LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER* LightProjDataPtr = (LIGHTVIEWPROJ_MATRIX_CONSTANT_BUFFER*)mappedLightProjResource.pData;
	*LightProjDataPtr = lightProjConstData;
	Context->Unmap(LightViewProjConstantBuffer, 0);

}

int DeferredRendering::getWinWidth()
{
	return WinWidth;
}

int DeferredRendering::getWinHeight()
{
	return WinHeight;
}

XMMATRIX DeferredRendering::getProjectionMatrix()
{
	return mProjection;
}

XMMATRIX DeferredRendering::getViewMatrix()
{
	return mView;
}

XMFLOAT4X4 DeferredRendering::getProjViewMatrix()
{
	XMFLOAT4X4 tmp;
	XMStoreFloat4x4(&tmp, projViewMatrix);
	return tmp;
}

DeferredRendering::~DeferredRendering()
{	
	GeoRSState->Release();
	LightBlendState->Release();
	SwapChain->Release();
	Device->Release();
	Context->Release();
	RTV->Release();
	DepthStencilBuffer->Release();
	DepthStencilView->Release();
	tmpRTV->Release();
	tmpSRV->Release();
	backbufferUAV->Release();
	FXAACS->Release();
	ShadowMapSamplerState->Release();
	ShadowDepthStencilBuffer->Release();
	SRVShadowMap->Release();
	LightVertexBuffer->Release();
	LightBoxVertexBuffer->Release();
	GeoVertexShader->Release();
	GeoVertexLayout->Release();
	GeoGeometryShader->Release();
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
	PointLightVertexShader->Release();
	PointLightPixelShader->Release();
	CamPSConstBuffer->Release();
	LightPSConstBuffer->Release();
	LightViewProjConstantBuffer->Release();
	PLWorldBuffer->Release();
	for (int i = 0; i < 4; i++)
		GBufferSRV[i]->Release();
	for (int i = 0; i < 4; i++)
		GBufferRTV[i]->Release();
}
