#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H
#include <iostream>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <vector>
#include<d3d11.h>
#include<windows.h>
#include <fstream>
#include <string.h>
#include<d3dcompiler.h>
#include "DeferredRendering.h"
using namespace DirectX;

struct HeightMapinfo {
	int terrainwidth; // the width of heightmap
	int terrainheight; // height of the heightmap
	XMFLOAT3 * heightMap; // Array to store terrain 's vertex positions
	int size;
	//	float * normal;



};
struct ray
{
	XMVECTOR origin;
	XMVECTOR direction;
};
struct TriangleVertex // en struct skapas f�r att lagra x y z kordinaterna f�r varje punkt
{
	XMFLOAT3 pos;
	XMFLOAT2 tex;
	XMFLOAT3 normal;
};
class Heightmap
{
public:
	Heightmap();
	void create(DeferredRendering &);
void Draw(ID3D11DeviceContext* &gContext);
float Heightmapcreater(char * filename, ID3D11Device* &gDevice);
void createtexture(ID3D11Device* &gDevice, char* ,ID3D11ShaderResourceView* & );
void Createflatmesh(ID3D11Device* &gDevice);
bool hiting(Camera &CamPos);
~Heightmap();
private:
	ID3D11PixelShader* gheightmapfragment = nullptr;
	ID3D11Buffer* gVertexBuffer;
	ID3D11Buffer* gIndexBuffer; 
	UINT32 offset = 0;
	UINT32 geoVertexSize = sizeof(float) * 8;
	ID3D11ShaderResourceView* gTextureView;
	ID3D11ShaderResourceView* gTextureView2;
	HeightMapinfo hminfo;
	int numfaces;
	ray rayen;
	std::vector<TriangleVertex> verticerna;
	std::vector<unsigned int> indicesna;
};







#endif