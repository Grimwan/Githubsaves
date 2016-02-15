#ifndef IMPORTOBJ_H
#define IMPORTOBJ_H


#include<iostream>
#include <string>
#include <vector>

using namespace std;


struct Vertice
{
	float posX, posY, posZ;
	float u, v;
	float norX, norY, norZ;
};

struct Color
{
	float r, g, b;
};




bool importObj(string file, vector<Vertice> &outVerticeList, wstring &textureFileName, Color &Ka, Color &Kd, Color &Ks, float &Ns);


bool importMtl(string file, wstring &textureFileName, Color &Ka, Color &Kd, Color &Ks, float &Ns);










#endif
