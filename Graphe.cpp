#include "Graphe.h"
#include <fstream>
#include <iostream>
#include <time.h>
#include <algorithm>
using namespace std;

void distanceMatrix(Graphe&); //calcule la matrice des distances de G
void BFS(Graphe&, int); //calcule une ligne de la matrice des distances à l'aide d'un dijkstra 
void cutminMatrix(Graphe&);
void cutmin(Graphe&, int, int);

void init(Graphe & g, string filename)
{
	clock_t t0;
	fstream f(filename, fstream::in);
	int i, j, n = 0;
	if (!f)
	{
		cout << "Fichier introuvable" << endl;
		g.n = 0;
		return;
	}
	f >> n;
	g.n = n;
	if (n <= 0)
	{
		cout << "Fichier invalide" << endl;
		return;
	}
	cout << "Lecture du graphe ... ";
	t0 = clock();
	g.A.resize(n);
	g.D.resize(n);
	g.cutmin.resize(n);
	g.voisins.resize(n);
	g.degre.resize(n);
	fill(g.degre.begin(), g.degre.end(), 0);
	for (i = 0; i < n; i++)
	{
		g.A[i].reset();
		g.D[i].resize(n);
		g.cutmin[i].resize(n);
		for (j = 0; j < n; j++)
		{
			g.D[i][j] = NMAX;
			g.cutmin[i][j] = NMAX;
		}
	}
	//on rempli la matrice d'adjacence
	for (i = 0; i < n; i++)
	{
		int k;
		f >> k;
		for (int m = 0; m < k; m++)
		{
			f >> j;
			g.A[i].set(j);
			g.voisins[i].push_back(j);
			g.degre[i]++;
		}
	}
	cout << "OK (" << (double)(clock() - t0) / (double)CLOCKS_PER_SEC << " sec.)" << endl;
	distanceMatrix(g);
}

void distanceMatrix(Graphe & g) //O(N^3)
{
	clock_t t0, t1;
	t0 = clock();
	cout << "Calcul de la matrice des distances ... " << endl;
	int n = g.n;
	cout << "0%" << flush;
	for (int k = 0; k < n; k++)
	{
		cout << "\r" << (100 * k) / n << "%" << flush;
		BFS(g, k);
	}
	cout << "\r100% (" << (double)(clock() - t0) / (double)CLOCKS_PER_SEC << " sec.)" << endl;
	t1 = clock();
}
void BFS(Graphe& g, int i) //version de l'algorithme de Dijkstra adaptée au cas des poids unitaires, complexité linéaire en nombre d'arêtes.
{
	list<int> temp[2];
	int index = 0, dist = 1;
	temp[index].push_back(i);
	g.D[i][i] = 0;
	while (!temp[index].empty())
	{
		index ^= 1;
		temp[index].clear();
		for (auto k = temp[index ^ 1].begin(); k != temp[index ^ 1].end(); k++)
			for (auto m = g.voisins[*k].begin(); m != g.voisins[*k].end(); m++)
				if (g.D[i][*m] == NMAX)
				{
					g.D[i][*m] = dist;
					temp[index].push_back(*m);
				}
		dist++;
	}
}

void cutminMatrix(Graphe &g)
{
	for (int i = 0; i < g.n; i++)
		for (int j = 0; j < g.n; j++)
			if (g.cutmin[j][i] == NMAX)
				cutmin(g, i, j);
			else
				g.cutmin[i][j] = g.cutmin[j][i];
	return;
}

void cutmin(Graphe &g, int i, int j)
{
	return;
}

