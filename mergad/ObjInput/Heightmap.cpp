#include "Heightmap.h"






void HeightmapLoad(char* filename, HeightMapinfo &hminfo)
{
	std::ofstream myfile;
	myfile.open("y.txt");
	FILE* f = fopen(filename, "rb");
	unsigned char header[54];
	fread(header, sizeof(unsigned char), 54, f);
	int index;
	unsigned char heeight;


	int width = *(int*)&header[18];
	int height = *(int*)&header[22];
	int size = 3 * width*height;

	unsigned char* datan = new unsigned char[size]; //Allocating 3bytes for each pixel. R G B
	fread(datan, 1, size, f);
	fclose(f);
	hminfo.terrainheight = height;
	hminfo.terrainwidth = width;

	hminfo.heightMap = new XMFLOAT3[hminfo.terrainwidth * hminfo.terrainheight];

	int k = 0;
	float heightfactor = 10.0f;  // A smoothness factor


	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			heeight = datan[k];
			index = (width*j) + i;
			hminfo.heightMap[index].x = (float)i;
			hminfo.heightMap[index].y = heeight / heightfactor;
			myfile << hminfo.heightMap[index].y << std::endl;
			hminfo.heightMap[index].z = (float)j;
			k += 3;
		}
	}
	hminfo.size = height*width;
	delete[] datan;
	datan = 0;
}

void Heightmap::Createflatmesh(ID3D11Device* &gDevice)
{
	int texUIndex = 0;
	int texVIndex = 0;
	int Width = hminfo.terrainwidth;
	int height = hminfo.terrainheight;
	int Antaletvertices = Width*height;
	int Numfaces = (Width - 1)*(height - 1) * 2;
	std::vector<TriangleVertex> vertices(Antaletvertices);
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < Width; i++)
		{
			vertices[j*Width + i].pos = hminfo.heightMap[j*Width + i];
		}

	}


	std::vector<unsigned int> indices(Numfaces * 3);
	int index = 0;
	for (int j = 0; j < height - 1; j++)
	{
		for (int i = 0; i < Width - 1; i++)
		{
			indices[index] = (1 + j)*Width + i; // 79 top left						

			indices[index + 1] = (1 + j)*Width + i + 1; // 80  top right

			indices[index + 2] = (j)*Width + i; // 0 bottom left

			indices[index + 3] = (1 + j)*Width + i + 1; // 80 top right

			indices[index + 4] = (j)*Width + i + 1; // 1 bottom right

			indices[index + 5] = (j)*Width + i; // 0 bottom left

			index += 6;

		}
	}

	indices;



	for (int i = 0; i < hminfo.terrainheight; i++)
	{
		for (int j = 0; j < hminfo.terrainwidth; j++)
		{
			vertices[j + hminfo.terrainwidth*i].tex = { float(j), float(i) };
		}
	}




	XMVECTOR normalen = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); //
	float vectoredge[3]; //The x and y and z values for vectormatris

	XMVECTOR* normallen = new XMVECTOR[hminfo.terrainheight*hminfo.terrainwidth];
	for (int i = 0;i < (hminfo.terrainheight*hminfo.terrainwidth);i++)
	{
		normallen[i] = XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	XMVECTOR edge[2];
	for (int i = 0; i < 2; i++)
	{
		edge[i] = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); //gives the edgeses for the zerovalue.
	}
	for (int i = 0; i < ((hminfo.terrainwidth - 1)*(hminfo.terrainheight - 1) * 2); ++i)
	{
		vectoredge[0] = vertices[indices[(i * 3)]].pos.x - vertices[indices[(i * 3) + 2]].pos.x;
		vectoredge[1] = vertices[indices[(i * 3)]].pos.y - vertices[indices[(i * 3) + 2]].pos.y;
		vectoredge[2] = vertices[indices[(i * 3)]].pos.z - vertices[indices[(i * 3) + 2]].pos.z;
		edge[0] = XMVectorSet(vectoredge[0], vectoredge[1], vectoredge[2], 0.0f);

		vectoredge[0] = vertices[indices[(i * 3) + 2]].pos.x - vertices[indices[(i * 3) + 1]].pos.x;
		vectoredge[1] = vertices[indices[(i * 3) + 2]].pos.y - vertices[indices[(i * 3) + 1]].pos.y;
		vectoredge[2] = vertices[indices[(i * 3) + 2]].pos.z - vertices[indices[(i * 3) + 1]].pos.z;
		edge[1] = XMVectorSet(vectoredge[0], vectoredge[1], vectoredge[2], 0.0f);  // vectors


		normallen[indices[(i * 3)]] = normallen[indices[i * 3]] + XMVector3Normalize(XMVector3Cross(edge[1], edge[0]));
		normallen[indices[(i * 3) + 1]] = (normallen[indices[(i * 3) + 1]] + XMVector3Cross(edge[1], edge[0]));
		normallen[indices[(i * 3) + 2]] = normallen[indices[(i * 3) + 2]] + XMVector3Cross(edge[1], edge[0]);

	}
	for (int i = 0;i < (hminfo.terrainheight*hminfo.terrainwidth);i++)
	{

		normallen[i] = XMVector3Normalize(normallen[i]);
		vertices[i].normal.x = XMVectorGetX(normallen[i]);
		vertices[i].normal.y = XMVectorGetY(normallen[i]);
		vertices[i].normal.z = XMVectorGetZ(normallen[i]);

	}
	verticerna = vertices;
	indicesna = indices;

	D3D11_BUFFER_DESC bufferDesc;

	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(TriangleVertex) * Antaletvertices;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);
	D3D11_BUFFER_DESC hm;
	memset(&hm, 0, sizeof(hm));
	hm.BindFlags = D3D11_BIND_INDEX_BUFFER;
	hm.Usage = D3D11_USAGE_DEFAULT;

	hm.ByteWidth = sizeof(unsigned int)* Numfaces * 3;
	D3D11_SUBRESOURCE_DATA hey;
	hey.pSysMem = &indices[0];

	hey.SysMemPitch = 0;
	hey.SysMemSlicePitch = 0;
	gDevice->CreateBuffer(&hm, &hey, &gIndexBuffer);

}

Heightmap::~Heightmap()
{
	gTextureView->Release();
	gTextureView2->Release();
	gVertexBuffer->Release();
	gIndexBuffer->Release();
}

void Heightmap::updateWorldBuffer(ID3D11DeviceContext *& gContext, ID3D11Buffer *& worldBuffer)
{
	WORLD_MATRIX_CONSTANT_BUFFER data;
	data.worldMatrix = worldMatrix;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	gContext->Map(worldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WORLD_MATRIX_CONSTANT_BUFFER* DataPtr = (WORLD_MATRIX_CONSTANT_BUFFER*)mappedResource.pData;
	*DataPtr = data;
	gContext->Unmap(worldBuffer, 0);
}

void Heightmap::createtexture(ID3D11Device* &gDevice, char* stuff, ID3D11ShaderResourceView* &gTextureViewen)
{
	int i;
	FILE* f = fopen(stuff, "rb");
	unsigned char header[54];
	fread(header, sizeof(unsigned char), 54, f);


	int width = *(int*)&header[18];
	int height = *(int*)&header[22];
	int size = 3 * width*height;

	unsigned char* datan = new unsigned char[size / 3 * 4];


	fread(datan, sizeof(unsigned char), size, f);
	fclose(f);
	unsigned char* tmpp = new unsigned char[size / 3 * 4];
	unsigned char* tmppp = datan;
	char bgr, brm;
	for (int i = 0; i < height / 2; i++)
	{
		for (int j = 0; j < width * 3; j++)
		{

			bgr = datan[j + width * 3 * i];
			brm = datan[j + width * 3 * (height - i)];
			datan[j + width * 3 * (height - i)] = bgr;
			datan[j + width * 3 * i] = brm;
		}



	}
	for (int i = 0; i < size; i += 3)
	{
		unsigned char tmp = datan[i];
		datan[i] = datan[i + 2];
		datan[i + 2] = tmp;

	}



	for (int i = 0; i < (size / 3); i++)
	{

		for (int j = 0; j < 3; j++)
		{
			tmpp[4 * i + j] = datan[3 * i + j];

		}

		tmpp[4 * i + 4] = 0;
	}
	datan = tmpp;


	D3D11_TEXTURE2D_DESC bthTexDesc;
	ZeroMemory(&bthTexDesc, sizeof(bthTexDesc));
	bthTexDesc.Width = width;
	bthTexDesc.Height = height;
	bthTexDesc.MipLevels = bthTexDesc.ArraySize = 1;
	bthTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	bthTexDesc.SampleDesc.Count = 1;
	bthTexDesc.SampleDesc.Quality = 0;
	bthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	bthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bthTexDesc.MiscFlags = 0;
	bthTexDesc.CPUAccessFlags = 0;

	//create texture from raw data (float*)
	ID3D11Texture2D  *pTexture = NULL;  //Pointer to the resource that will serve as input to a shader. This resource must have been created with the D3D11_BIND_SHADER_RESOURCE flag. 
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)datan;
	data.SysMemPitch = width * 4 * sizeof(char);
	gDevice->CreateTexture2D(&bthTexDesc, &data, &pTexture);

	// resource view description 
	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	ZeroMemory(&resViewDesc, sizeof(resViewDesc));
	resViewDesc.Format = bthTexDesc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MipLevels = bthTexDesc.MipLevels;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	gDevice->CreateShaderResourceView(pTexture, &resViewDesc, &gTextureViewen);
	pTexture->Release(); // releaseing tge texture

}


void Heightmap::createtextureWIC(ID3D11Device* &gDevice, wchar_t* fileName, ID3D11ShaderResourceView* &gTextureViewen)
{
	ID3D11Resource* texture;
	CreateWICTextureFromFile(gDevice, fileName, &texture, &gTextureViewen);
	texture->Release();
}


bool Heightmap::hiting(Camera &camera)
{

	rayen.origin = camera.getCamPos();
	rayen.direction = XMVECTOR{ 0,-1,0 };
	XMFLOAT3 temp;
	XMStoreFloat3(&temp, rayen.origin);
	//Putting camera in grid local space
	temp.x += ((float)hminfo.terrainwidth)*0.5;
	temp.z += ((float)hminfo.terrainheight)*0.5;

	SHORT OKey = GetAsyncKeyState('O');
	SHORT PKey = GetAsyncKeyState('P');


	int x, z;
	x = temp.x;
	z = temp.z;

	if ((x + 1<hminfo.terrainwidth) && (z + 1<hminfo.terrainheight) && (x >= 0) && (z >= 0))
	{
		//New algorithm
		float yValue;
		float bot, top;
		float xLocal, zLocal;

		xLocal = temp.x - x;
		zLocal = temp.z - z;

		bot = verticerna[x + (z * hminfo.terrainwidth)].pos.y * (1 - xLocal) + verticerna[(x + 1) + (z * hminfo.terrainwidth)].pos.y * xLocal;
		top = verticerna[x + ((z + 1) * hminfo.terrainwidth)].pos.y * (1 - xLocal) + verticerna[(x + 1) + ((z + 1) * hminfo.terrainwidth)].pos.y * xLocal;

		yValue = bot * (1 - zLocal) + top * zLocal;

		camera.setCamPosY(yValue + 6);


		return true;
	}
	return false;

}

Heightmap::Heightmap()
{
	gVertexBuffer = nullptr;
	gIndexBuffer = nullptr;
	gTextureView = nullptr;
	gTextureView2 = nullptr;
	gTextureView3 = nullptr;
	numfaces = 0;
	rayen.direction = { 0,-1,0, };
	rayen.origin = { 0,0,0,0 };
}
void Heightmap::buffer(ID3D11Device* &gDevice, int width, int height)
{
	PS_CONSTANT_BUFFER_heightmap laddain;
	laddain.height = height;
	laddain.width = width;

	D3D11_BUFFER_DESC HeightMapbufferDesc;
	ZeroMemory(&HeightMapbufferDesc, sizeof(HeightMapbufferDesc));
	HeightMapbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HeightMapbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	HeightMapbufferDesc.CPUAccessFlags = 0;
	HeightMapbufferDesc.ByteWidth = sizeof(PS_CONSTANT_BUFFER_heightmap);

	D3D11_SUBRESOURCE_DATA pData;
	ZeroMemory(&pData, sizeof(D3D11_SUBRESOURCE_DATA));
	pData.pSysMem = &laddain;

	gDevice->CreateBuffer(&HeightMapbufferDesc, &pData, &HeightmapConstBuffer);
}

void Heightmap::create(DeferredRendering &defferend)
{
	ID3DBlob* pPS = nullptr;
	HRESULT hr;

	ID3DBlob* errorBlob;

	hr = D3DCompileFromFile(L"heightmapfragment.hlsl", nullptr, nullptr, "PS_main", "ps_4_0", 0, 0, &pPS, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	defferend.Device->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gheightmapfragment);
	pPS->Release();
}
void Heightmap::Draw(ID3D11DeviceContext* &gContext, ID3D11Buffer* &worldBuffer)
{
	updateWorldBuffer(gContext, worldBuffer);

	//heightmapbuffers
	gContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &geoVertexSize, &offset);
	gContext->IASetIndexBuffer(gIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	gContext->PSSetConstantBuffers(1, 1, &HeightmapConstBuffer);
	gContext->PSSetSamplers(0, 1, &samplerState);
	gContext->PSSetShaderResources(0, 1, &gTextureView);
	gContext->PSSetShaderResources(1, 1, &gTextureView2);
	gContext->PSSetShaderResources(2, 1, &gTextureView3);
	gContext->PSSetShader(gheightmapfragment, nullptr, 0);
	//heightmap
	gContext->DrawIndexed(numfaces * 3, 0, 0);
}

float Heightmap::Heightmapcreater(char* filename, ID3D11Device* &gDevice)
{




	HeightmapLoad(filename, hminfo);
	numfaces = float((hminfo.terrainheight - 1)*(hminfo.terrainwidth - 1) * 2);
	Createflatmesh(gDevice);
	createtextureWIC(gDevice, L"grass.bmp", gTextureView); // skapar texturen
	createtextureWIC(gDevice, L"Rock.JPG", gTextureView2);
	createtextureWIC(gDevice, L"heightmap.bmp", gTextureView3);
	buffer(gDevice, hminfo.terrainwidth, hminfo.terrainheight);
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	gDevice->CreateSamplerState(&samplerDesc, &samplerState);

	XMMATRIX tmpWorldMatrix = XMMatrixTranslation(-hminfo.terrainwidth*0.5, 0.0f, -hminfo.terrainheight*0.5);
	tmpWorldMatrix = XMMatrixTranspose(tmpWorldMatrix);
	XMStoreFloat4x4(&worldMatrix, tmpWorldMatrix);

	return numfaces;
}
