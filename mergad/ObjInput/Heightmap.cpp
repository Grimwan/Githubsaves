#include "Heightmap.h"




struct HeightMapinfo {
	int terrainwidth; // the width of heightmap
	int terrainheight; // height of the heightmap
	XMFLOAT3 * heightMap; // Array to store terrain 's vertex positions
	int size;
	//	float * normal;



};
struct TriangleVertex // en struct skapas f�r att lagra x y z kordinaterna f�r varje punkt
{
	XMFLOAT3 pos;
	XMFLOAT2 tex;
	XMFLOAT3 normal;
};


void HeightmapLoad(char* filename, HeightMapinfo &hminfo)
{
	FILE* f = fopen(filename, "rb"); // �ppnar bmp filen och rb �r vilket mod den �ppnas i r st�r f�r reading och b st�r f�r bin�r. 
	unsigned char header[54]; // h�r lagras headern utav bmp filen. 
	fread(header, sizeof(unsigned char), 54, f); // l�ser in 54 bites headern till headern. 
	int index;
	unsigned char heeight;

	//extraherar h�jden coh breden fr�n headerfilen.
	int width = *(int*)&header[18]; // &header = adressen fr�n headern vi jobbar i headern och vid int* s� tar vi adressen vi f�tt och pekare till int.  }http://stackoverflow.com/questions/19168294/what-does-int-mean-in-c
									// tar du det som �r p� adressen som en pekare s� blir det en int. typecastar samtidigt till int
	int height = *(int*)&header[22];
	// samma som ovan. 
	//	int size = (3 * width + 3) & (~3);
	int size = 3 * width*height; // 3 kommer ifr�n rgb // Mikael.
								 //	int size2 = (size / 3) * 4;
	unsigned char* datan = new unsigned char[size]; // man allokerar 3 bytes per pixel inom datan. 
	fread(datan, 1, size, f);  // fread  tar (var datan lagras(buffern), storleken p� itemsen i bytes, maximalt nummer utav items som ska l�sas, h�r �r pointern till fil strukturen. 
	fclose(f); // st�nger ner datan. 
	hminfo.terrainheight = height;
	hminfo.terrainwidth = width;
	hminfo.heightMap = new XMFLOAT3[hminfo.terrainwidth * hminfo.terrainheight];
	//	hminfo.normal = new float[hminfo.terrainheight * hminfo.terrainwidth];

	int k = 0;
	float heightfactor = 5.0f;  // f�r att inte g�ra det f�r spikigt


	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			heeight = datan[k];
			index = (width*j) + i;
			hminfo.heightMap[index].x = (float)i;
			//			hminfo.heightMap[index].y = 0;
			hminfo.heightMap[index].y = heeight / heightfactor;
			hminfo.heightMap[index].z = (float)j;
			//			hminfo.normal[index] = (float)heeight;
			k += 3;
		}
	}
	hminfo.size = height*width;
	delete[] datan;
	datan = 0;
}



void Createflatmesh(HeightMapinfo &hminfo, std::vector<TriangleVertex> &swag, std::vector<unsigned int> &yolo, ID3D11Buffer* &gVertexBuffer, ID3D11Buffer* &gIndexBuffer, ID3D11Device* &gDevice)
{
	std::ifstream file;
	file.open("normal.txt");
	float a, b, c;
	

	//	float* grayscale =new float[hminfo.terrainheight*hminfo.terrainwidth];
	int texUIndex = 0;
	int texVIndex = 0;
	int Width = hminfo.terrainwidth;
	int height = hminfo.terrainheight;
	int Antaletvertices = Width*height;
	int Numfaces = (Width - 1)*(height - 1) * 2;
	std::vector<TriangleVertex> triangleVertices(Antaletvertices);
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < Width; i++)
		{
			file >> a >> b >> c;
			triangleVertices[j*Width + i].pos = hminfo.heightMap[j*Width + i];
			triangleVertices[j*Width + i].normal = XMFLOAT3(a, b, c);
			//		grayscale[j*Width + i] = hminfo.normal[j*Width + i];
			triangleVertices[j*Width + i].tex = XMFLOAT2(((1 / float(Width - 1))*i), ((float(height - 1 - j) / float(height - 1))));
			//	triangleVertices[j*Width + i].tex = XMFLOAT2(1, 1);
			//	texUIndex = ((1 / Width)*i);
		}
		//	texUIndex = 0;
		//	texVIndex = ((Width - j) / Width);
	}
	swag = triangleVertices;
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

			indices[index] = (1 + j)*Width + i; // 79 h�gst upp v�nster /top left
												//	triangleVertices[(j + 1)*Width + 1].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 0.0f);

			indices[index + 1] = (1 + j)*Width + i + 1; // 80 h�gst upp h�ger top right
														//	triangleVertices[(j + 1)*Width + j + 1].tex = XMFLOAT2(texUIndex + 1.0f, texVIndex + 0.0f);

			indices[index + 2] = (j)*Width + i; // 0 l�ngst ner v�nster bottom left
												//	triangleVertices[j*Width + i].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 1.0f);




			indices[index + 3] = (1 + j)*Width + i + 1; // 80 h�gst upp h�ger top right
														//		triangleVertices[(j + 1)*Width + j + 1].tex = XMFLOAT2(texUIndex + 1.0f, texVIndex + 0.0f);

			indices[index + 4] = (j)*Width + i + 1; // 1 l�ngst ner h�ger bottom right
													//		triangleVertices[j*Width + i].tex = XMFLOAT2(texUIndex + 0.0f, texVIndex + 1.0f);

			indices[index + 5] = (j)*Width + i; // 0 l�ngst ner v�nster bottom left
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

	yolo = indices;



	// http://www.braynzarsoft.net/viewtutorial/q16390-30-heightmap-terrain
	D3D11_BUFFER_DESC bufferDesc;   // buffer recoursen beskriver buffern

	memset(&bufferDesc, 0, sizeof(bufferDesc));   // fyller minnet i bufferdesc med 0 och best�mmer antal bytes som s�tts till 0.  
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; //h�r best�mmer vi att den ska bindas till vertexbuffern. 
	bufferDesc.Usage = D3D11_USAGE_DEFAULT; // h�r best�mms hur buffern ska l�sas till coh skrivas till eftersom den �r default s� st�r det f�r att den kan l�sas och skrivas till genom gpun.
	bufferDesc.ByteWidth = sizeof(TriangleVertex) * Antaletvertices; // h�r best�ms hur stor buffern �r.

	D3D11_SUBRESOURCE_DATA data; // h�r skappas subresourcen �ndrar p� bufferdescen. 
	data.pSysMem = &triangleVertices[0]; //h�r pekas v�rdet till var datan startades.
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer); //h�r skapas sj�lva buffern  f�rst �r en pointer som beskriver buffern, sedan en pekare till datan som startar programmet[inputen],h�r �r adressen till pointern f�r f�r interfacet f�r sj�lva det skapade buffer objektet.

															   // h�r skapas index liten och textur kordinaterna l�gs till.

															   ////	std::vector<unsigned int> indices();
															   //	triangleVertices[Width].tex = XMFLOAT2(0.0f,  0.0f);
															   //	triangleVertices[Width + 1].tex = XMFLOAT2(1.0f, 0.0f);
															   //	triangleVertices[1].tex = XMFLOAT2(1.0f, 1.0f);
															   //	triangleVertices[0].tex = XMFLOAT2( 0.0f, 1.0f);
															   //
															   //	unsigned int indices[] = { 79,80,0,  80,1,0};
															   //	


	D3D11_BUFFER_DESC hm;   // buffer recoursen beskriver buffern
	memset(&hm, 0, sizeof(hm));   // fyller minnet i bufferdesc med 0 och best�mmer antal bytes som s�tts till 0.  
	hm.BindFlags = D3D11_BIND_INDEX_BUFFER; //h�r best�mmer vi att den ska bindas till vertexbuffern. 
	hm.Usage = D3D11_USAGE_DEFAULT; // h�r best�mms hur buffern ska l�sas till coh skrivas till eftersom den �r default s� st�r det f�r att den kan l�sas och skrivas till genom gpun.
									//	hm.ByteWidth = sizeof(indices); // h�r best�ms hur stor buffern �r.
	hm.ByteWidth = sizeof(unsigned int)* Numfaces * 3;
	D3D11_SUBRESOURCE_DATA hey; // h�r skappas subresourcen �ndrar p� bufferdescen. 
	hey.pSysMem = &indices[0]; //h�r pekas v�rdet till var datan startades.
							   //	hey.pSysMem = indices;
	hey.SysMemPitch = 0;
	hey.SysMemSlicePitch = 0;
	gDevice->CreateBuffer(&hm, &hey, &gIndexBuffer); //h�r skapas sj�lva 
	file.close();
}

void createtexture(ID3D11Device* &gDevice, ID3D11ShaderResourceView* &gTextureView)
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
	data.SysMemPitch = BTH_IMAGE_WIDTH * 4 * sizeof(char);  // hur l�ng en rad �r s� den vet n�r den ska b�rja p� n�sta
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
	FILE* f = fopen("rainbow.bmp", "rb"); // �ppnar bmp filen och rb �r vilket mod den �ppnas i r st�r f�r reading och b st�r f�r bin�r. 
	unsigned char header[54]; // h�r lagras headern utav bmp filen. 
	fread(header, sizeof(unsigned char), 54, f); // l�ser in 54 bites headern till headern. 

												 //extraherar h�jden coh breden fr�n headerfilen.
	int width = *(int*)&header[18]; // &header = adressen fr�n headern vi jobbar i headern och vid int* s� tar vi adressen vi f�tt och pekare till int.  }http://stackoverflow.com/questions/19168294/what-does-int-mean-in-c
									// tar du det som �r p� adressen som en pekare s� blir det en int. typecastar samtidigt till int
	int height = *(int*)&header[22];
	// samma som ovan. 
	//	int size = (3 * width + 3) & (~3);
	int size = 3 * width*height; // 3 kommer ifr�n rgb // Mikael.
								 //	int size2 = (size / 3) * 4;
	unsigned char* datan = new unsigned char[size / 3 * 4]; // man allokerar 3 bytes per pixel inom datan. 
	fread(datan, sizeof(unsigned char), size, f);  // fread  tar (var datan lagras(buffern), storleken p� itemsen i bytes, maximalt nummer utav items som ska l�sas, h�r �r pointern till fil strukturen. 
	fclose(f); // st�nger ner datan. 
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
	for (int i = 0; i < size; i += 3)   // lagrar alla datav�rden som nu �r colors dock �r det i ordningen bgr s� ordningen skriver jag om h�r som rgb
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
	data.SysMemPitch = width * 4 * sizeof(char);  // hur l�ng en rad �r s� den vet n�r den ska b�rja p� n�sta
	gDevice->CreateTexture2D(&bthTexDesc, &data, &pTexture);

	// resource view description 
	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	ZeroMemory(&resViewDesc, sizeof(resViewDesc));
	resViewDesc.Format = bthTexDesc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MipLevels = bthTexDesc.MipLevels;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	gDevice->CreateShaderResourceView(pTexture, &resViewDesc, &gTextureView);
	pTexture->Release(); // releaseing tge texture

}


float heigthmapcreater(ID3D11Buffer* &gVertexBuffer, ID3D11Buffer* &gIndexBuffer, ID3D11Device* &gDevice, ID3D11ShaderResourceView* &gTextureView)
{
	//	CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout
	HeightMapinfo hminfo;
	HeightmapLoad("nigas.bmp", hminfo);
	std::vector<TriangleVertex> Vertices(hminfo.terrainheight*hminfo.terrainwidth);
	std::vector<unsigned int> Indices(((hminfo.terrainwidth - 1)*(hminfo.terrainheight - 1) * 2) * 3);
	Createflatmesh(hminfo, Vertices, Indices, gVertexBuffer, gIndexBuffer, gDevice);
	createtexture(gDevice, gTextureView); // skapar texturen
	return (float((hminfo.terrainheight - 1)*(hminfo.terrainwidth - 1) * 2));
}

