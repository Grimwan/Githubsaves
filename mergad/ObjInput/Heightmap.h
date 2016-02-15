#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H
#include <iostream>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <vector>
#include<d3d11.h>
#include<windows.h>
#include <fstream>

using namespace DirectX;



float heigthmapcreater(ID3D11Buffer* &gVertexBuffer, ID3D11Buffer* &gIndexBuffer, ID3D11Device* &gDevice, ID3D11ShaderResourceView* &gTextureView);











#endif