#include"createDirectx.h"
#include <iostream>
#include<d3d11.h>

int gSampleCount = 1;

void InitDirect3D(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext, IDXGISwapChain* &gSwapChain, ID3D11RenderTargetView* &gRTV, ID3D11Texture2D* &gDepthStencilBuffer, ID3D11DepthStencilView* &gDepthStencilView, ID3D11UnorderedAccessView* &backbufferUAV, HWND & winHandle, int width, int height)
{
	InitialiseWindow(winHandle, width, height);
	CreateDeviceAndSwapChain(gDevice, gContext, gSwapChain, winHandle, width, height);
	CreateRenderTargetView(gDevice, gContext, gSwapChain, gRTV, gDepthStencilBuffer, gDepthStencilView, backbufferUAV, width, height);
	CreateViewport(gContext, width, height);
}


void InitialiseWindow(HWND & winHandle, int width, int height)
{
	HINSTANCE applicationHandle = GetModuleHandle(NULL);
	WNDCLASS windowClass;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = applicationHandle;
	windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = L"WindowClass";

	RegisterClass(&windowClass);

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	winHandle = CreateWindow(L"WindowClass", L"Project DV1542", WS_OVERLAPPEDWINDOW, 100, 50, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, applicationHandle, NULL);

	ShowWindow(winHandle, SW_SHOWDEFAULT);
	UpdateWindow(winHandle);
}



LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(handle, message, wParam, lParam);
}




void CreateDeviceAndSwapChain(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext, IDXGISwapChain* &gSwapChain, HWND &winHandle, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Width = width;
	scDesc.BufferDesc.Height = height;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.RefreshRate.Denominator = 0;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.BufferCount = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	scDesc.SampleDesc.Count = gSampleCount;
	scDesc.SampleDesc.Quality = 0;
	scDesc.OutputWindow = winHandle;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = 0;

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, nullptr, 0, D3D11_SDK_VERSION, &scDesc, &gSwapChain, &gDevice, nullptr, &gContext);

}

void CreateDepthStencil(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext,ID3D11Texture2D* &gDepthStencilBuffer, ID3D11DepthStencilView* &gDepthStencilView, int width, int height)
{
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = gSampleCount;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	gDevice->CreateTexture2D(&depthStencilDesc, NULL, &gDepthStencilBuffer);
	gDevice->CreateDepthStencilView(gDepthStencilBuffer, NULL, &gDepthStencilView);
}

void CreateRenderTargetView(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext, IDXGISwapChain* &gSwapChain, ID3D11RenderTargetView* &gRTV, ID3D11Texture2D* &gDepthStencilBuffer, ID3D11DepthStencilView* &gDepthStencilView, ID3D11UnorderedAccessView* &backbufferUAV, int width, int height)
{
	ID3D11Texture2D* backBuffer;
	gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRTV);


	HRESULT hr = gDevice->CreateUnorderedAccessView(backBuffer, NULL, &backbufferUAV);
	if (FAILED(hr))
		std::cout << "Failed to ceate backbuffer UAV" << std::endl;

	backBuffer->Release();

	CreateDepthStencil(gDevice, gContext, gDepthStencilBuffer, gDepthStencilView, width, height);

}


void CreateViewport(ID3D11DeviceContext* &gContext, int width, int height)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;

	gContext->RSSetViewports(1, &vp);
}




