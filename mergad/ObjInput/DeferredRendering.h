#ifndef DEFERREDRENDERING_H
#define DEFERREDRENDERING_H

#include "SimpleMath.h"
#include "SimpleMath.inl"
#include <d3dcompiler.h>
#include "Camera.h"
#include "importObj.h"
#include "createDirectx.h"
using namespace DirectX;

class DeferredRendering
{
public:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* Context = nullptr;
	IDXGISwapChain* SwapChain = nullptr;
	DeferredRendering(Camera &camera);
	void SetGeoPass();
	void SetLightPass();
	void SetShadowMapPass();
	void UpdateFrame(Camera &camera);
	ID3D11Buffer* GeoPSConstBuffer = { nullptr };
	ID3D11Buffer* WorldBuffer = { nullptr };
	~DeferredRendering();
	int getWinHeight();
	int getWinWidth();
	XMMATRIX getProjectionMatrix();
	XMMATRIX getViewMatrix();
	HWND WinHandle = NULL;
private:
	//basic directx resources
	ID3D11RenderTargetView* RTV = nullptr;
	ID3D11Texture2D* DepthStencilBuffer = nullptr;
	ID3D11DepthStencilView* DepthStencilView = nullptr;
	int WinWidth = 1200;
	int WinHeight = 950;
	//Deferred rendering
	void CreateShaders();
	void CreateBuffers(Camera &camera);
	void CreateDepthBufferShadowMap();
	void CreateLightVertexBuffer();
	DeferredRendering();
	//Create deferred rendering and shadow map resources
	ID3D11Texture2D* ShadowDepthStencilBuffer = { nullptr };
	ID3D11ShaderResourceView* SRVShadowMap = { nullptr };
	ID3D11Buffer* LightVertexBuffer = { nullptr };
	int nrOfSamples;
	//Geo pass resources
	ID3D11VertexShader* GeoVertexShader = {nullptr};
	ID3D11InputLayout* GeoVertexLayout = { nullptr };
	ID3D11PixelShader* GeoPixelShader = { nullptr };
	ID3D11Buffer* ProjViewBuffer = { nullptr };
	//Shadow map light pass resources
	ID3D11DepthStencilView* DepthStencilViewShadowMap = { nullptr };
	ID3D11VertexShader* LightShadowVertexShader = { nullptr };
	ID3D11InputLayout* LightShadowVertexLayout = nullptr;
	//Light pass resources
	ID3D11VertexShader* LightVertexShader = { nullptr };
	ID3D11InputLayout* LightVertexLayout = { nullptr };
	ID3D11PixelShader* LightPixelShader = { nullptr };
	ID3D11Buffer* CamPSConstBuffer = { nullptr };
	ID3D11Buffer* LightPSConstBuffer = { nullptr };
	ID3D11Buffer* LightViewProjConstantBuffer = { nullptr };
	ID3D11ShaderResourceView* GBufferSRV[4] = { nullptr };
	ID3D11RenderTargetView* GBufferRTV[4] = { nullptr };
	UINT32 lightVertexSize;
	UINT32 lightOffset;
	XMMATRIX mView, mProjection, projViewMatrix, viewMatrix;
	float clearColor[4];
};
#endif
