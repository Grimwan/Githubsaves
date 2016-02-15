#ifndef CREATEDIRECTX_H
#define CREATEDIRECTX_H

#include<Windows.h>
#include<d3d11.h>



void InitialiseWindow(HWND & winHandle, int width, int height);
LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

void InitDirect3D(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext, IDXGISwapChain* &gSwapChain, ID3D11RenderTargetView* &gRTV, ID3D11Texture2D* &gDepthStencilBuffer, ID3D11DepthStencilView* &gDepthStencilView, HWND &winHandle, int width, int height);
void CreateDeviceAndSwapChain(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext, IDXGISwapChain* &gSwapChain, HWND &winHandle, int width, int height);
void CreateRenderTargetView(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext, IDXGISwapChain* &gSwapChain, ID3D11RenderTargetView* &gRTV ,ID3D11Texture2D* &gDepthStencilBuffer, ID3D11DepthStencilView* &gDepthStencilView, int width, int height);
void CreateViewport(ID3D11DeviceContext* &gContext, int width, int height);
void CreateDepthStencil(ID3D11Device* &gDevice, ID3D11DeviceContext* &gContext, ID3D11Texture2D* &gDepthStencilBuffer, ID3D11DepthStencilView* &gDepthStencilView, int width, int height);








#endif
