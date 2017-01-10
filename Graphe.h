#include<string>
#include<vector>
#ifndef DEF_H_GRAPHE
#define DEF_H_GRAPHE
using namespace std;
typedef unsigned __int32 uint32_t;

struct Graphe
{
	uint32_t* A; //matrice d'adjacence étendue A[k][i][j] est (A^(k+1))_{ij}, A[0] est donc la matrice d'adjacence. On aura en plus A[n-1]=A* (taille n^3)
	vector<vector<int>> a_liste; //adjacence sous forme de liste de voisins, pour accelerer les produits par A, principalement.
	int n; //dimension de la matrice carrée
	int k_max; //le plus grand k pour lequel on connais A[k];
} ;
void init(Graphe & g,string filename); //entre autres alloue A
void end(Graphe & g); // désalloue A
void print(Graphe const & g,int k); // affiche A[k], affiche A* si k>=n-1
#endif