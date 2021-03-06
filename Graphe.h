#include<string>
#include<vector>
#include <bitset>
#include <list>
#ifndef DEF_H_GRAPHE
#define DEF_H_GRAPHE
#define NMAX 2000
using namespace std;
typedef unsigned __int32 uint32_t;

struct Graphe
{
	vector<bitset<NMAX>> A; //matrice d'adjacence (sym�trique)
	vector<vector<unsigned short>> D;//matrice de  n-distance de G
	vector<list<int>> voisins;
	int n; //dimension de la matrice carr�e
} ;
void init(Graphe & g,string filename); 
#endif