#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include"importObj.h"

using namespace std;


struct TexCoord
{
	float u, v;
};


struct Normal
{
	float x, y, z;
};

struct Vec
{
	float	x, y, z;
	Vec() {};
	Vec(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};
	float Dot(const Vec& v2) { return (x * v2.x) + (y * v2.y) + (z * v2.z); }

	Vec operator-(const Vec& v)
	{
		return Vec(this->x - v.x, this->y - v.y, this->z - v.z);
	}

	Vec operator+(const Vec& v)
	{
		return Vec(this->x + v.x, this->y + v.y, this->z + v.z);
	}

	Vec operator*(const float& f)
	{
		return Vec(this->x * f, this->y * f, this->z * f);
	}

	
	inline float Length2()
	{
		return (x*x + y*y + z*z);
	}

	float Length()
	{
		return sqrt(Length2());
	}

	void Normalize()
	{
		float l = Length();
		x /= l;
		y /= l;
		z /= l;
	}

	Vec Cross(const Vec &v)
	{
		return Vec(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}
};



bool importObj(string file, vector<Vertice> &outVerticeList, wstring &textureFileName, Color &Ka, Color &Kd, Color &Ks, float &Ns)
{
	vector<Vec>verticeList(0);
	vector<TexCoord>texCoordsList(0);
	vector<Normal>normalsList(0);
	ifstream fileStream;

	string mtlFileName;

	fileStream.open(file.c_str());

	if (!fileStream.is_open())
	{
		cout << "Fail" << endl;
		return false;
	}

	string line;
	while (!fileStream.eof()) // shiiiit
	{

		string test;
		fileStream >> test;

		float x, y, z;


		if (test[0] == '#')
		{
			getline(fileStream, line);
		}
		else if (test[0] == 'm')
		{
			if (!test.compare("mtllib"))
			{
				fileStream >> mtlFileName;
				importMtl(mtlFileName, textureFileName, Ka, Kd, Ks, Ns);
			}
		}
		else if (test[0] == 'u')
		{
			if (!test.compare("usemtl"))
			{
				fileStream >> mtlFileName;
			}
		}
		else if (test[0] == 'f')
		{
			Vertice outVertice;

			vector<int> space;
			vector<int> slash;

			getline(fileStream,line);


			while (line[line.size() - 1] == ' ')
			{
				line.pop_back();
			}


			for (int i = 0; i < line.size(); i++)
			{
				if (line[i] == '/')
				{
					slash.push_back(i);
				}
				else if (line[i] == ' ')
				{
					space.push_back(i);
				}
			}

			if (space.size() == 3)
			{
				
				if (slash.size() == 0)
				{
	
				}
				else if (slash.size() == 3)
				{
					int vertexIndex[3];
					int texCoordsIndex[3];


					for (int i = 0; i < 2; i++)
					{
						vertexIndex[i] = stoi(line.substr(space[i] + 1, slash[i] - space[i] - 1));
						texCoordsIndex[i] = stoi(line.substr(slash[i] + 1, space[i+1] - slash[i] - 1));
					}

					vertexIndex[2] = stoi(line.substr(space[2] + 1, slash[2] - space[2] - 1));
					texCoordsIndex[2] = stoi(line.substr(slash[2] + 1, line.size() - slash[2] - 1));
					
					//Calculating the normal
					Vec v1 = verticeList[vertexIndex[1]-1] - verticeList[vertexIndex[0]-1];
					Vec v2 = verticeList[vertexIndex[2]-1] - verticeList[vertexIndex[0]-1];
					Vec normal = v1.Cross(v2);
					normal.Normalize();


					//Giving value to vertice list
					for (int i = 0; i < 3; i++)
					{
						outVertice.posX = verticeList[vertexIndex[i] - 1].x;
						outVertice.posY = verticeList[vertexIndex[i] - 1].y;
						outVertice.posZ = verticeList[vertexIndex[i] - 1].z;

						outVertice.u = texCoordsList[texCoordsIndex[i] - 1].u;
						outVertice.v = texCoordsList[texCoordsIndex[i] - 1].u;

						outVertice.norX = normal.x;
						outVertice.norY = normal.y;
						outVertice.norZ = normal.z;

						outVerticeList.push_back(outVertice);
					}
				}
				else if (slash.size() == 6)
				{
					int vertexIndex[3];
					int texCoordsIndex[3];
					int normalsIndex[3];


					if (slash[1] - slash[0] == 1)
					{
						
					}
					else
					{
						for (int i = 0; i < 2; i++)
						{
							vertexIndex[i] = stoi(line.substr(space[i] + 1, slash[2*i] - space[i] - 1))-1;
							texCoordsIndex[i] = stoi(line.substr(slash[2 * i] + 1, slash[2 * i +1] - slash[2 * i] - 1))-1;
							normalsIndex[i] = stoi(line.substr(slash[2 * i+1] + 1, space[i+1] - slash[2 * i+1] - 1))-1;
						}		
						vertexIndex[2] = stoi(line.substr(space[2] + 1, slash[4] - space[2] - 1))-1;
						texCoordsIndex[2] = stoi(line.substr(slash[4] + 1, slash[5] - slash[4] - 1))-1;
						normalsIndex[2] = stoi(line.substr(slash[5] + 1, line.size() - slash[5] - 1))-1;


						for (int i = 0; i < 3; i++)
						{
							outVertice.posX = verticeList[vertexIndex[i]].x;
							outVertice.posY = verticeList[vertexIndex[i]].y;
							outVertice.posZ = verticeList[vertexIndex[i]].z;

							outVertice.u = texCoordsList[texCoordsIndex[i]].u;
							outVertice.v = texCoordsList[texCoordsIndex[i]].v;

							outVertice.norX = normalsList[normalsIndex[i]].x;
							outVertice.norY = normalsList[normalsIndex[i]].y;
							outVertice.norZ = normalsList[normalsIndex[i]].z;

							outVerticeList.push_back(outVertice);
						}

					}
				
					
				}
			}
			else if (space.size() == 4)
			{
				if (slash.size() == 0)
				{

				}
				else if (slash.size() == 4)
				{

					int vertexIndex[4];
					int texCoordsIndex[4];


					for (int i = 0; i < 3; i++)
					{
						vertexIndex[i] = stoi(line.substr(space[i] + 1, slash[i] - space[i] - 1));
						texCoordsIndex[i] = stoi(line.substr(slash[i] + 1, space[i + 1] - slash[i] - 1));
					}

					vertexIndex[3] = stoi(line.substr(space[3] + 1, slash[3] - space[3] - 1));
					texCoordsIndex[3] = stoi(line.substr(slash[3] + 1, line.size() - slash[3] - 1));

					//Calculating the normal
					Vec v1 = verticeList[vertexIndex[1]-1] - verticeList[vertexIndex[0]-1];
					Vec v2 = verticeList[vertexIndex[2]-1] - verticeList[vertexIndex[0]-1];
					Vec normal = v1.Cross(v2);
					normal.Normalize();


					for (int i = 0; i < 3; i++)
					{
						outVertice.posX = verticeList[vertexIndex[i] - 1].x;
						outVertice.posY = verticeList[vertexIndex[i] - 1].y;
						outVertice.posZ = verticeList[vertexIndex[i] - 1].z;

						outVertice.u = texCoordsList[texCoordsIndex[i] - 1].u;
						outVertice.v = texCoordsList[texCoordsIndex[i] - 1].u;

						outVertice.norX = normal.x;
						outVertice.norY = normal.y;
						outVertice.norZ = normal.z;

						outVerticeList.push_back(outVertice);
					}
					//first itteration
					outVertice.posX = verticeList[vertexIndex[2] - 1].x;
					outVertice.posY = verticeList[vertexIndex[2] - 1].y;
					outVertice.posZ = verticeList[vertexIndex[2] - 1].z;

					outVertice.u = texCoordsList[texCoordsIndex[2] - 1].u;
					outVertice.v = texCoordsList[texCoordsIndex[2] - 1].u;

					outVertice.norX = normal.x;
					outVertice.norY = normal.y;
					outVertice.norZ = normal.z;

					outVerticeList.push_back(outVertice);
					//second itteration
					outVertice.posX = verticeList[vertexIndex[1] - 1].x;
					outVertice.posY = verticeList[vertexIndex[1] - 1].y;
					outVertice.posZ = verticeList[vertexIndex[1] - 1].z;

					outVertice.u = texCoordsList[texCoordsIndex[1] - 1].u;
					outVertice.v = texCoordsList[texCoordsIndex[1] - 1].u;

					outVertice.norX = normal.x;
					outVertice.norY = normal.y;
					outVertice.norZ = normal.z;

					outVerticeList.push_back(outVertice);
					//third itteration
					outVertice.posX = verticeList[vertexIndex[3] - 1].x;
					outVertice.posY = verticeList[vertexIndex[3] - 1].y;
					outVertice.posZ = verticeList[vertexIndex[3] - 1].z;

					outVertice.u = texCoordsList[texCoordsIndex[3] - 1].u;
					outVertice.v = texCoordsList[texCoordsIndex[3] - 1].u;

					outVertice.norX = normal.x;
					outVertice.norY = normal.y;
					outVertice.norZ = normal.z;

					outVerticeList.push_back(outVertice);



				}
				else if (slash.size() == 8)
				{
					int vertexIndex[4];
					int texCoordsIndex[4];
					int normalsIndex[4];


					if (slash[1] - slash[0] == 1)
					{

					}
					else
					{
						for (int i = 0; i < 3; i++)
						{
							vertexIndex[i] = stoi(line.substr(space[i] + 1, slash[2 * i] - space[i] - 1));
							texCoordsIndex[i] = stoi(line.substr(slash[2 * i] + 1, slash[2 * i + 1] - slash[2 * i] - 1));
							normalsIndex[i] = stoi(line.substr(slash[2 * i + 1] + 1, space[i + 1] - slash[2 * i + 1] - 1));
						}
						vertexIndex[3] = stoi(line.substr(space[3] + 1, slash[6] - space[3] - 1));
						texCoordsIndex[3] = stoi(line.substr(slash[6] + 1, slash[7] - slash[6] - 1));
						normalsIndex[3] = stoi(line.substr(slash[7] + 1, line.size() - slash[7] - 1));

						for (int i = 0; i < 3; i++)
						{
							outVertice.posX = verticeList[vertexIndex[i] - 1].x;
							outVertice.posY = verticeList[vertexIndex[i] - 1].y;
							outVertice.posZ = verticeList[vertexIndex[i] - 1].z;

							outVertice.u = texCoordsList[texCoordsIndex[i] - 1].u;
							outVertice.v = texCoordsList[texCoordsIndex[i] - 1].u;

							outVertice.norX = normalsList[normalsIndex[i] - 1].x;
							outVertice.norY = normalsList[normalsIndex[i] - 1].y;
							outVertice.norZ = normalsList[normalsIndex[i] - 1].z;

							outVerticeList.push_back(outVertice);
						}
					}
				}
			}

		}
		else if (test[0] == 'v')
		{
			if (test.size() == 1)
			{
				fileStream >> x >> y >> z;
				verticeList.push_back({ x,y,z });
			}
			else if (test[1] == 'n')
			{
				fileStream >> x >> y >> z;
				normalsList.push_back({ x,y,z });
			}
			else if (test[1] == 't')
			{
				fileStream >> x >> y;
				texCoordsList.push_back({ x,y });
			}

		}

	}
	fileStream.close();


	//for (int i = 0; i < verticeList.size(); i++)
	//{
	//	cout << verticeList[i].x << " " << verticeList[i].y << " " << verticeList[i].z << endl;
	//}
	//for (int i = 0; i < normalsList.size(); i++)
	//{
	//	cout << normalsList[i].x << " " << normalsList[i].y << " " << normalsList[i].z << endl;
	//}


	return true;
}








bool importMtl(string file, wstring &textureFileName, Color &Ka, Color &Kd, Color &Ks, float &Ns)
{
	ifstream fileStream;
	fileStream.open(file.c_str());
	while (!fileStream.eof())
	{
		float r, g, b;
		string test;
		fileStream >> test;
		if (test[0] == 'K')
		{
			if (test[1] == 'a')
			{
				fileStream >> r >> g >> b;
				Ka = { r, g, b };
			}
			else if (test[1] == 'd')
			{
				fileStream >> r >> g >> b;
				Kd = { r, g, b };
			}
			else if (test[1] == 's')
			{
				fileStream >> r >> g >> b;
				Ks = { r, g, b };
			}
		}
		else if (test[0] == 'N')
		{
			if (test[1] == 's')
			{
				fileStream >> Ns;
			}
		}
		else if (!test.compare("map_Kd"))
		{
			string temp;
			getline(fileStream,temp);
			while (temp[0] == ' ')
			{
				temp.erase(temp.begin());
			}
	
			for (int i = 0; i < temp.size(); i++)
			{
				textureFileName += wchar_t(temp[i]);
			}
			
		}
		else
		{
			getline(fileStream, test);
		}
	}


	return true;
}