#include<string>
#include<vector>
#include <bitset>
#ifndef DEF_H_GRAPHE
#define DEF_H_GRAPHE
#define NMAX 600
using namespace std;
typedef unsigned __int32 uint32_t;

struct Graphe
{
	vector<bitset<NMAX>> A; //matrice d'adjacence (symétrique)
	vector<vector<unsigned short>> D;//matrice de  n-distance de G
	int n; //dimension de la matrice carrée
} ;
void init(Graphe & g,string filename); 
#endif