#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

int main()
{
	struct Pos
	{
		float x, y, z;
	};
	vector<Pos>pos(0);

	struct TexCoord
	{
		float u, v;
	};
	vector<TexCoord>texCoord(0);

	struct Normal
	{
		float normalX, normalY, normalZ;
	};
	vector<Normal>normal(0);

	struct Vertice
	{
		float posX, posY, posZ;
		float u, v;
		float norX, norY, norZ;
	};


	//xyz data1;
	//uv data2;
	//normal data3;
	//int numbers;
	ifstream Vert;
	Vert.open("file.txt");
	//int size;
	string line;
	while (!Vert.eof()) // shiiiit
	{
		char test;
		Vert >> test;

		float x, y, z;


		if (test == '#')
		{
			getline(Vert, line);
		}
		else if (test == 'f')
		{

			vector<int> space;
			vector<int> slash;

			getline(Vert, line);


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
					int xyz[3];
					x = stoi(line.substr(space[0] + 1, space[1] - space[0] - 1));
					y = stoi(line.substr(space[1] + 1, space[2] - space[1] - 1));
					z = stoi(line.substr(space[2] + 1, line.size() - space[1] - 2));
					cout << x << " " << y << " " << z << endl;
				}
				else if (slash.size() == 3)
				{
					int xyz[3];
					int txyz[3];


					for (int i = 0; i < 2; i++)
					{
						xyz[i] = stoi(line.substr(space[i] + 1, slash[i] - space[i] - 1));
						txyz[i] = stoi(line.substr(slash[i] + 1, space[i + 1] - slash[i] - 1));
					}

					xyz[2] = stoi(line.substr(space[2] + 1, slash[2] - space[2] - 1));
					txyz[2] = stoi(line.substr(slash[2] + 1, line.size() - slash[2] - 1));

					for (int i = 0; i < 3; i++)
					{
						cout << xyz[i] << "/" << txyz[i] << " ";
					}
					cout << endl;

				}
				else if (slash.size() == 6)
				{
					int xyz[3];
					int txyz[3];
					int nxyz[3];


					if (slash[1] - slash[0] == 1)
					{

					}
					else
					{
						for (int i = 0; i < 2; i++)
						{
							xyz[i] = stoi(line.substr(space[i] + 1, slash[2 * i] - space[i] - 1));
							txyz[i] = stoi(line.substr(slash[2 * i] + 1, slash[2 * i + 1] - slash[2 * i] - 1));
							nxyz[i] = stoi(line.substr(slash[2 * i + 1] + 1, space[i + 1] - slash[2 * i + 1] - 1));
						}
						xyz[2] = stoi(line.substr(space[2] + 1, slash[4] - space[2] - 1));
						txyz[2] = stoi(line.substr(slash[4] + 1, slash[5] - slash[4] - 1));
						nxyz[2] = stoi(line.substr(slash[5] + 1, line.size() - slash[5] - 1));

						for (int i = 0; i < 3; i++)
						{
							cout << xyz[i] << "/" << txyz[i] << "/" << nxyz[i] << " ";
						}
						cout << endl;
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

				}
				else if (slash.size() == 8)
				{

				}
			}

		}
		else if (test == 'v')
		{
			Vert >> test;
			if (test == 'n')
			{
				Vert >> x >> y >> z;
				normal.push_back({ x,y,z });
			}
			else if (test == 'p')
			{
				Vert >> x >> y >> z;
				pos.push_back({ x,y,z });
			}
			else if (test == 't')
			{
				Vert >> x >> y;
				texCoord.push_back({ x,y });
			}

		}

	}
	Vert.close();


	for (int i = 0; i < pos.size(); i++)
	{
		cout << pos[i].x << " " << pos[i].y << " " << pos[i].z << endl;
	}


	getchar();
	return 0;
}
