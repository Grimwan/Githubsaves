#ifndef DEFERREDRENDERING_H
#define DEFERREDRENDERING_H

#include "SimpleMath.h"
#include "SimpleMath.inl"
#include <d3dcompiler.h>
#include "Camera.h"
#include "importObj.h"
#include "createDirectx.h"
#include "Structs.h"
using namespace DirectX;

class DeferredRendering
{
public:
	HWND WinHandle = NULL;
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* Context = nullptr;
	IDXGISwapChain* SwapChain = nullptr;
	DeferredRendering(Camera &camera);
	void SetGeoPass();
	void SetLightPass();
	void SetShadowMapPass();
	void UpdateFrame(Camera &camera);
	void setFXAA();
	ID3D11Buffer* GeoPSConstBuffer = { nullptr };
	ID3D11Buffer* WorldBuffer = { nullptr };
	int getWinWidth();
	int getWinHeight();
	XMMATRIX getProjectionMatrix();
	XMMATRIX getViewMatrix();
	XMFLOAT4X4 getProjViewMatrix();
	~DeferredRendering();
private:
	//BlendStates
	ID3D11BlendState* LightBlendState = { nullptr };
	//FXAA resources
	ID3D11RenderTargetView* tmpRTV = nullptr;
	ID3D11ShaderResourceView* tmpSRV = nullptr ;
	ID3D11UnorderedAccessView * backbufferUAV = nullptr;
	ID3D11ComputeShader* FXAACS = nullptr;
	//basic directx resources
	ID3D11RenderTargetView* RTV = nullptr;
	ID3D11Texture2D* DepthStencilBuffer = nullptr;
	ID3D11DepthStencilView* DepthStencilView = nullptr;
	int WinWidth = 1200;
	int WinHeight = 950;
	//Deferred rendering
	void CreateShaders();
	void CreateComputeShader();
	void CreateBuffers(Camera &camera);
	void CreateDepthBufferShadowMap();
	void CreateLightVertexBuffer();
	void CreateBlendStates();
	void CreatePointLightShaders();
	void CreatePointLightVertexBuffer();
	void CreatePLWorldBuffer();
	void CreateLight();
	void CreateRSStates();
	void UpdateWorldBuffer(ID3D11Buffer* &worldBuffer, XMFLOAT4X4 worldMatrix);
	DeferredRendering();
	
	ID3D11RasterizerState* GeoRSState = nullptr;

	//Create deferred rendering and shadow map resources
	ID3D11SamplerState* ShadowMapSamplerState;
	ID3D11Texture2D* ShadowDepthStencilBuffer = { nullptr };
	ID3D11ShaderResourceView* SRVShadowMap = { nullptr };
	ID3D11Buffer* LightVertexBuffer = { nullptr };
	ID3D11Buffer* LightBoxVertexBuffer = { nullptr };
	D3D11_VIEWPORT ScreenViewPort;
	XMVECTOR shadowMapOrigin;
	int nrOfSamples;
	//Geo pass resources
	ID3D11VertexShader* GeoVertexShader = {nullptr};
	ID3D11InputLayout* GeoVertexLayout = { nullptr };
	ID3D11GeometryShader* GeoGeometryShader = { nullptr };
	ID3D11PixelShader* GeoPixelShader = { nullptr };
	ID3D11Buffer* ProjViewBuffer = { nullptr };
	//Shadow map light pass resources
	ID3D11DepthStencilView* DepthStencilViewShadowMap = { nullptr };
	ID3D11VertexShader* LightShadowVertexShader = { nullptr };
	ID3D11InputLayout* LightShadowVertexLayout = nullptr;
	D3D11_VIEWPORT ShadowMapViewPort;
	//Light pass resources
	ID3D11RasterizerState* LightRSState = nullptr; 
	ID3D11VertexShader* LightVertexShader = { nullptr };
	ID3D11InputLayout* LightVertexLayout = { nullptr };
	ID3D11PixelShader* LightPixelShader = { nullptr };
	ID3D11VertexShader* PointLightVertexShader = { nullptr };
	ID3D11PixelShader* PointLightPixelShader = { nullptr };
	ID3D11Buffer* CamPSConstBuffer = { nullptr };
	ID3D11Buffer* LightPSConstBuffer = { nullptr };
	ID3D11Buffer* LightViewProjConstantBuffer = { nullptr };
	ID3D11ShaderResourceView* GBufferSRV[4] = { nullptr };
	ID3D11RenderTargetView* GBufferRTV[4] = { nullptr };
	UINT32 lightVertexSize;
	UINT32 lightOffset;
	PointLight pointLight;
	ID3D11Buffer* PLWorldBuffer;

	XMMATRIX mView, mProjection, projViewMatrix, mLightView, mLightProjection, lightProjViewMatrix;
	float clearColor[4];
};
#endif
