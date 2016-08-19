#include "Heightmap.h"






void HeightmapLoad(char* filename, HeightMapinfo &hminfo)
{
	std::ofstream myfile;
	myfile.open("y.txt");
	FILE* f = fopen(filename, "rb"); // öppnar bmp filen och rb är vilket mod den öppnas i r står för reading och b står för binär. 
	unsigned char header[54]; // här lagras headern utav bmp filen. 
	fread(header, sizeof(unsigned char), 54, f); // läser in 54 bites headern till headern. 
	int index;
	unsigned char heeight;

	//extraherar höjden coh breden från headerfilen.
	int width = *(int*)&header[18]; // &header = adressen från headern vi jobbar i headern och vid int* så tar vi adressen vi fått och pekare till int.  }http://stackoverflow.com/questions/19168294/what-does-int-mean-in-c
									// tar du det som är på adressen som en pekare så blir det en int. typecastar samtidigt till int
	int height = *(int*)&header[22];
	// samma som ovan. 
	//	int size = (3 * width + 3) & (~3);
	int size = 3 * width*height; // 3 kommer ifrån rgb // Mikael.
								 //	int size2 = (size / 3) * 4;
	unsigned char* datan = new unsigned char[size]; // man allokerar 3 bytes per pixel inom datan. 
	fread(datan, 1, size, f);  // fread  tar (var datan lagras(buffern), storleken på itemsen i bytes, maximalt nummer utav items som ska läsas, här är pointern till fil strukturen. 
	fclose(f); // stänger ner datan. 
	hminfo.terrainheight = height;
	hminfo.terrainwidth = width;

	hminfo.heightMap = new XMFLOAT3[hminfo.terrainwidth * hminfo.terrainheight];
	//	hminfo.normal = new float[hminfo.terrainheight * hminfo.terrainwidth];

	int k = 0;
	float heightfactor = 10.0f;  // för att inte göra det för spikigt


	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			heeight = datan[k];
			index = (width*j) + i;
			hminfo.heightMap[index].x = (float)i;
			//			hminfo.heightMap[index].y = 0;
			hminfo.heightMap[index].y = heeight / heightfactor;
			myfile << hminfo.heightMap[index].y << std::endl;
			hminfo.heightMap[index].z = (float)j;
			//			hminfo.normal[index] = (float)heeight;
			k += 3;
		}
	}
	hminfo.size = height*width;
	delete[] datan;
	datan = 0;
}

void Heightmap::Createflatmesh(ID3D11Device* &gDevice)
{
	//	std::ifstream file;
	//	file.open("normal.txt");
	//	float a, b, c;
	//	float* grayscale =new float[hminfo.terrainheight*hminfo.terrainwidth];
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
			//		file >> a >> b >> c;
			vertices[j*Width + i].pos = hminfo.heightMap[j*Width + i];
			//			

			//			triangleVertices[j*Width + i].normal = XMFLOAT3(a, b, c);
			//		grayscale[j*Width + i] = hminfo.normal[j*Width + i];
			//		vertices[j*Width + i].tex = XMFLOAT2(((1 / float(Width - 1))*i), ((float(height - 1 - j) / float(height - 1))));
			//	triangleVertices[j*Width + i].tex = XMFLOAT2(1, 1);
			//	texUIndex = ((1 / Width)*i);
		}
		//	texUIndex = 0;
		//	texVIndex = ((Width - j) / Width);
	}





	//TriangleVertex triangleVertices[6241];
	//for (int j = 0; j < height; j++)
	//{
	//	for (int i = 0; i < Width; i++)
	//	{
	//		triangleVertices[j*Width + i].pos = hminfo.heightMap[j*Width + i];
	//	//	triangleVertices[j*cols + i].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//		grayscale[j*Width + i] = hminfo.normal[j*Width + i];
	//		
	//		triangleVertices[j*Width + i].tex = XMFLOAT2(1, 1);
	//	}

	//}
	//
	//	unsigned int indices[] = { 79,80,0,  80,1,0,  80,81,1,   81,2,1};

	std::vector<unsigned int> indices(Numfaces * 3);
	int index = 0;
	for (int j = 0; j < height - 1; j++)
	{
		for (int i = 0; i < Width - 1; i++)
		{

			indices[index] = (1 + j)*Width + i; // 79 högst upp vänster /top left
												//	triangleVertices[(j + 1)*Width + 1].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 0.0f);

			indices[index + 1] = (1 + j)*Width + i + 1; // 80 högst upp höger top right
														//	triangleVertices[(j + 1)*Width + j + 1].tex = XMFLOAT2(texUIndex + 1.0f, texVIndex + 0.0f);

			indices[index + 2] = (j)*Width + i; // 0 längst ner vänster bottom left
												//	triangleVertices[j*Width + i].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 1.0f);




			indices[index + 3] = (1 + j)*Width + i + 1; // 80 högst upp höger top right
														//		triangleVertices[(j + 1)*Width + j + 1].tex = XMFLOAT2(texUIndex + 1.0f, texVIndex + 0.0f);

			indices[index + 4] = (j)*Width + i + 1; // 1 längst ner höger bottom right
													//		triangleVertices[j*Width + i].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 1.0f);

			indices[index + 5] = (j)*Width + i; // 0 längst ner vänster bottom left
												//		triangleVertices[j*Width + i].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 1.0f);

												//		    triangleVertices[(j + 1)*Width + 1].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 0.0f);
												//			triangleVertices[(j + 1)*Width + j + 1].tex = XMFLOAT2(texUIndex + 1.0f, texVIndex + 0.0f);
												//			triangleVertices[(j + 1)*Width + i].tex = XMFLOAT2(texUIndex + 1.0f, texVIndex + 1.0f);
												//			triangleVertices[j*Width + i].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 1.0f);





			index += 6;
			//		texUIndex++;
		}
		//	texUIndex = 0;
		//	texVIndex++;
	}

	indices;



	for (int i = 0; i < hminfo.terrainheight; i++)
	{
		for (int j = 0; j < hminfo.terrainwidth; j++)
		{
			vertices[j + hminfo.terrainwidth*i].tex = { float(j), float(i) };
		}
	}




	//	std::ifstream file;
	//	file.open("fungerandenormal");
	//		myfile.open("normal.txt");
	//	std::vector<XMFLOAT3> tempnormal;
	XMVECTOR normalen = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); //
	float vectoredge[3]; // här lagras x och y och z värdena för vectormatrisen

	XMVECTOR* normallen = new XMVECTOR[hminfo.terrainheight*hminfo.terrainwidth];
	for (int i = 0;i < (hminfo.terrainheight*hminfo.terrainwidth);i++)
	{
		normallen[i] = XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	//std::vector<XMVECTOR> verticesstoring(hminfo.terrainheight*hminfo.terrainwidth);
	XMVECTOR edge[2];
	for (int i = 0; i < 2; i++)
	{
		edge[i] = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); // ger edgesena nollvärden;
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
		edge[1] = XMVectorSet(vectoredge[0], vectoredge[1], vectoredge[2], 0.0f);  // vektorn


		normallen[indices[(i * 3)]] = normallen[indices[i * 3]] + XMVector3Normalize(XMVector3Cross(edge[1], edge[0]));
		normallen[indices[(i * 3) + 1]] = (normallen[indices[(i * 3) + 1]] + XMVector3Cross(edge[1], edge[0]));
		normallen[indices[(i * 3) + 2]] = normallen[indices[(i * 3) + 2]] + XMVector3Cross(edge[1], edge[0]);

		//	normalen
		//	XMStoreFloat3(&onormaliserad, XMVector3Cross(edge[0], edge[1])); //XMvector3cross är crossmultiplikation inbyggt XMStorefloat3 först var lagras datan, sedan vector var datan finns.
		//	XMVector3Normalize(onormaliserad);
		//		tempnormal.push_back(onormaliserad); //lagrar normalen på tempnormal vectorn ochs gör vectorn större. 
	}
	for (int i = 0;i < (hminfo.terrainheight*hminfo.terrainwidth);i++)
	{

		normallen[i] = XMVector3Normalize(normallen[i]);
		vertices[i].normal.x = XMVectorGetX(normallen[i]);
		vertices[i].normal.y = XMVectorGetY(normallen[i]);
		vertices[i].normal.z = XMVectorGetZ(normallen[i]);

		/*	vertices[i].normal.x = 0;
		vertices[i].normal.y = 0;
		vertices[i].normal.z = 0;

		*/
		//		file >> vertices[i].normal.x >> vertices[i].normal.y >> vertices[i].normal.z;
		//			myfile << XMVectorGetX(normallen[i]) << " " << XMVectorGetY(normallen[i]) << " " << XMVectorGetZ(normallen[i]) << std::endl;
	}



	verticerna = vertices;
	indicesna = indices;



	// http://www.braynzarsoft.net/viewtutorial/q16390-30-heightmap-terrain
	D3D11_BUFFER_DESC bufferDesc;   // buffer recoursen beskriver buffern

	memset(&bufferDesc, 0, sizeof(bufferDesc));   // fyller minnet i bufferdesc med 0 och bestämmer antal bytes som sätts till 0.  
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; //här bestämmer vi att den ska bindas till vertexbuffern. 
	bufferDesc.Usage = D3D11_USAGE_DEFAULT; // här bestämms hur buffern ska läsas till coh skrivas till eftersom den är default så står det för att den kan läsas och skrivas till genom gpun.
	bufferDesc.ByteWidth = sizeof(TriangleVertex) * Antaletvertices; // här bestäms hur stor buffern är.

	D3D11_SUBRESOURCE_DATA data; // här skappas subresourcen ändrar på bufferdescen. 
	data.pSysMem = &vertices[0]; //här pekas värdet till var datan startades.
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer); //här skapas själva buffern  först är en pointer som beskriver buffern, sedan en pekare till datan som startar programmet[inputen],här är adressen till pointern för för interfacet för själva det skapade buffer objektet.

															   // här skapas index liten och textur kordinaterna lägs till.

															   ////	std::vector<unsigned int> indices();
															   //	triangleVertices[Width].tex = XMFLOAT2(0.0f,  0.0f);
															   //	triangleVertices[Width + 1].tex = XMFLOAT2(1.0f, 0.0f);
															   //	triangleVertices[1].tex = XMFLOAT2(1.0f, 1.0f);
															   //	triangleVertices[0].tex = XMFLOAT2( 0.0f, 1.0f);
															   //
															   //	unsigned int indices[] = { 79,80,0,  80,1,0};
															   //	


	D3D11_BUFFER_DESC hm;   // buffer recoursen beskriver buffern
	memset(&hm, 0, sizeof(hm));   // fyller minnet i bufferdesc med 0 och bestämmer antal bytes som sätts till 0.  
	hm.BindFlags = D3D11_BIND_INDEX_BUFFER; //här bestämmer vi att den ska bindas till vertexbuffern. 
	hm.Usage = D3D11_USAGE_DEFAULT; // här bestämms hur buffern ska läsas till coh skrivas till eftersom den är default så står det för att den kan läsas och skrivas till genom gpun.
									//	hm.ByteWidth = sizeof(indices); // här bestäms hur stor buffern är.
	hm.ByteWidth = sizeof(unsigned int)* Numfaces * 3;
	D3D11_SUBRESOURCE_DATA hey; // här skappas subresourcen ändrar på bufferdescen. 
	hey.pSysMem = &indices[0]; //här pekas värdet till var datan startades.
							   //	hey.pSysMem = indices;
	hey.SysMemPitch = 0;
	hey.SysMemSlicePitch = 0;
	gDevice->CreateBuffer(&hm, &hey, &gIndexBuffer); //här skapas själva 
													 //	file.close();
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
	/*

	D3D11_TEXTURE2D_DESC bthTexDesc;
	ZeroMemory(&bthTexDesc, sizeof(bthTexDesc)); // fills a block of memeory with zeros. first is destination othr is its length.
	bthTexDesc.Width = BTH_IMAGE_WIDTH;
	bthTexDesc.Height = BTH_IMAGE_HEIGHT;
	bthTexDesc.MipLevels = bthTexDesc.ArraySize = 1;
	bthTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Red 8abit Green 8bit Blue 8 bit A alfa bit= genomskinlighhet
	bthTexDesc.SampleDesc.Count = 1;
	bthTexDesc.SampleDesc.Quality = 0;
	bthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	bthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // needed for pTexture
	bthTexDesc.MiscFlags = 0;
	bthTexDesc.CPUAccessFlags = 0;

	//create texture from raw data (float*)
	ID3D11Texture2D  *pTexture = NULL;  //Pointer to the resource that will serve as input to a shader. This resource must have been created with the D3D11_BIND_SHADER_RESOURCE flag.
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)BTH_IMAGE_DATA;
	data.SysMemPitch = BTH_IMAGE_WIDTH * 4 * sizeof(char);  // hur lång en rad är så den vet när den ska börja på nästa
	gDevice->CreateTexture2D(&bthTexDesc, &data, &pTexture);

	// resource view description
	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	ZeroMemory(&resViewDesc,sizeof(resViewDesc));
	resViewDesc.Format = bthTexDesc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MipLevels = bthTexDesc.MipLevels;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	gDevice->CreateShaderResourceView(pTexture, &resViewDesc, &gTextureView);
	pTexture->Release(); // releaseing tge texture
	*/


	int i;
	//	FILE* f = fopen("rainbow.bmp", "rb"); // öppnar bmp filen och rb är vilket mod den öppnas i r står för reading och b står för binär. 
	FILE* f = fopen(stuff, "rb");
	unsigned char header[54]; // här lagras headern utav bmp filen. 
	fread(header, sizeof(unsigned char), 54, f); // läser in 54 bites headern till headern. 

												 //extraherar höjden coh breden från headerfilen.
	int width = *(int*)&header[18]; // &header = adressen från headern vi jobbar i headern och vid int* så tar vi adressen vi fått och pekare till int.  }http://stackoverflow.com/questions/19168294/what-does-int-mean-in-c
									// tar du det som är på adressen som en pekare så blir det en int. typecastar samtidigt till int
	int height = *(int*)&header[22];
	// samma som ovan. 
	//	int size = (3 * width + 3) & (~3);
	int size = 3 * width*height; // 3 kommer ifrån rgb // Mikael.
								 //	int size2 = (size / 3) * 4;
	unsigned char* datan = new unsigned char[size / 3 * 4]; // man allokerar 3 bytes per pixel inom datan. 


	fread(datan, sizeof(unsigned char), size, f);  // fread  tar (var datan lagras(buffern), storleken på itemsen i bytes, maximalt nummer utav items som ska läsas, här är pointern till fil strukturen. 
	fclose(f); // stänger ner datan. 
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
	for (int i = 0; i < size; i += 3)   // lagrar alla datavärden som nu är colors dock är det i ordningen bgr så ordningen skriver jag om här som rgb
	{
		unsigned char tmp = datan[i];
		datan[i] = datan[i + 2];
		datan[i + 2] = tmp;

	}

	//for (int i = 0; i < size; i++)
	//{
	//	tmpp[i] = datan[size -1- i];
	//}
	//datan = tmpp;
	//tmpp = tmppp;
	//tmppp = datan;

	for (int i = 0; i < (size / 3); i++) // swag
	{

		for (int j = 0; j < 3; j++)
		{
			tmpp[4 * i + j] = datan[3 * i + j];

		}

		tmpp[4 * i + 4] = 0;
	}
	datan = tmpp;

	/*
	for (int i = 0; i < size; i++)
	{
	for (int j = 0; j < 4; j++)
	{
	tmpp[j + i] = datan[size - i  + j - 3];

	}
	i=i + 3;
	}
	datan = tmpp;*/

	D3D11_TEXTURE2D_DESC bthTexDesc;
	ZeroMemory(&bthTexDesc, sizeof(bthTexDesc)); // fills a block of memeory with zeros. first is destination othr is its length. 
	bthTexDesc.Width = width;
	bthTexDesc.Height = height;
	bthTexDesc.MipLevels = bthTexDesc.ArraySize = 1;
	bthTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// Red 8abit Green 8bit Blue 8 bit A alfa bit= genomskinlighhet
												   //	bthTexDesc.Format = DXGI_FORMAT_R8G8B8
	bthTexDesc.SampleDesc.Count = 1;
	bthTexDesc.SampleDesc.Quality = 0;
	bthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	bthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // needed for pTexture
	bthTexDesc.MiscFlags = 0;
	bthTexDesc.CPUAccessFlags = 0;

	//create texture from raw data (float*)
	ID3D11Texture2D  *pTexture = NULL;  //Pointer to the resource that will serve as input to a shader. This resource must have been created with the D3D11_BIND_SHADER_RESOURCE flag. 
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)datan;
	data.SysMemPitch = width * 4 * sizeof(char);  // hur lång en rad är så den vet när den ska börja på nästa
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

	//if (OKey)
	//	cout << temp.x << endl;
	//if (PKey)
	//	cout << temp.z << endl;
	int x, z;
	x = temp.x;
	z = temp.z;

	if ((x + 1<hminfo.terrainwidth) && (z + 1<hminfo.terrainheight) && (x >= 0) && (z >= 0))
	{

		//float xlength[3], zlength[3], lengthvector[3];
		//float length, quadcordinateX, quadcordinateZ, extra1, extra2, yvalue;
		//extra1 = quadcordinateX = temp.x - x;
		//extra2 = quadcordinateZ = temp.z - z;
		//length = sqrt(pow(quadcordinateX, 2) + pow(quadcordinateZ, 2));
		//quadcordinateX = quadcordinateX / length;
		//quadcordinateZ = quadcordinateZ / length;

		//if (quadcordinateX > 0.5)
		//{
		//	xlength[0] = extra1 - 0;
		//	zlength[0] = extra2 - 1;
		//	lengthvector[0] = sqrt(pow(xlength[0], 2) + pow(zlength[0], 2));
		//	xlength[1] = extra1 - 1;
		//	zlength[1] = extra2 - 1;
		//	lengthvector[1] = sqrt(pow(xlength[1], 2) + pow(zlength[1], 2));
		//	xlength[2] = extra1 - 0;
		//	zlength[2] = extra2 - 0;
		//	lengthvector[2] = sqrt(pow(xlength[2], 2) + pow(zlength[2], 2));
		//	length = lengthvector[0] + lengthvector[1] + lengthvector[2];



		//	yvalue = (lengthvector[0] / length) * verticerna[x + (z + 1)*hminfo.terrainwidth].pos.y + (lengthvector[1] / length) * verticerna[x + 1 + (z + 1)*hminfo.terrainwidth].pos.y + (lengthvector[2] / length) * verticerna[x + z*hminfo.terrainwidth].pos.y;

		//}
		//else
		//{


		//	xlength[0] = extra1 - 1;
		//	zlength[0] = extra2 - 1;
		//	lengthvector[0] = sqrt(pow(xlength[0], 2) + pow(zlength[0], 2));
		//	xlength[1] = extra1 - 1;
		//	zlength[1] = extra2 - 0;
		//	lengthvector[1] = sqrt(pow(xlength[1], 2) + pow(zlength[1], 2));
		//	xlength[2] = extra1 - 0;
		//	zlength[2] = extra2 - 0;
		//	lengthvector[2] = sqrt(pow(xlength[2], 2) + pow(zlength[2], 2));
		//	length = lengthvector[0] + lengthvector[1] + lengthvector[2];



		//	yvalue = (lengthvector[0] / length) * verticerna[x + 1 + (z + 1)*hminfo.terrainwidth].pos.y + (lengthvector[1] / length) * 	verticerna[x + 1 + z*hminfo.terrainwidth].pos.y + (lengthvector[2] / length) * verticerna[x + z*hminfo.terrainwidth].pos.y;


		//}


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
