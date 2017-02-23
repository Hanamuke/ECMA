#include "Graphe.h"
#include <fstream>
#include <iostream>
#include <time.h>
#include <algorithm>
using namespace std;

void distanceMatrix(Graphe&); //calcule la matrice des distances de G
void BFS(Graphe&, int); //calcule une ligne de la matrice des distances à l'aide d'un dijkstra 
void cutminMatrix(Graphe&);
int FFA(Graphe const&, int, int, vector<bool>&);

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
	g.C.resize(n);
	g.voisins.resize(n);
	g.degre.resize(n);
	fill(g.degre.begin(), g.degre.end(), 0);
	for (i = 0; i < n; i++)
	{
		g.A[i].reset();
		g.D[i].resize(n);
		g.C[i].resize(n);
		for (j = 0; j < n; j++)
		{
			g.D[i][j] = NMAX;
			g.C[i][j] = NMAX;
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
	cutminMatrix(g);
}

void distanceMatrix(Graphe & g) //O(N^3)
{
	clock_t t0;
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

void cutminMatrix(Graphe &g) //O(n^4)
{
	vector<bool> sol;
	vector<pair<unsigned int, unsigned int>> Gomory_Hu; //couple place dans la chaîne de Gomory-Hu// poids de l'arrete sortante à droite.
	Gomory_Hu.resize(g.n);
	clock_t t0;
	t0 = clock();
	cout << "Calcul de la matrice des coupes minimales ... " << endl;
	cout << "0%" << flush;
	for (int i = 0; i < g.n; i++)
	{
		Gomory_Hu[i].first = 0;
		Gomory_Hu[i].second = 0xffffffff;
	}
	for (int e = 0; e < g.n - 1;)
	{
		cout << "\r" << (100 * e) / g.n << "%" << flush;
		int s = -1, t = -1;//sommets à séparer, tous deux de l'ensemble s
		for (int k = 0; k < g.n; k++) //recherche des deux sommets à séparer
			if (Gomory_Hu[k].first == e)
				if (s == -1)
					s = k;
				else
				{
					t = k;
					break;
				}
		if (t == -1) //si l'ensemble était déjà séparé
		{
			e++;
			continue;
		}
		int val = FFA(g, s, t, sol), cnt = 0; //on trouve la coupe min séparant i et j, O(n^3)
		for (int k = 0; k < g.n; k++)
			if (Gomory_Hu[k].first == e && sol[k])
			{
				Gomory_Hu[k].second = val;
				cnt++;
			}
		for (int k = 0; k < g.n; k++)
			if (Gomory_Hu[k].first == e && !sol[k])
				Gomory_Hu[k].first += cnt;
		for (int i = 0; i < g.n; i++)
			for (int j = i; j < g.n; j++)
				if (sol[i] != sol[j])
					g.C[i][j] = g.C[i][j] <= val ? g.C[i][j] : val;
	}
	for (int i = 0; i < g.n; i++)
		for (int j = i + 1; j < g.n; j++)
			g.C[j][i] = g.C[i][j];
	cout << "\r100% (" << (double)(clock() - t0) / (double)CLOCKS_PER_SEC << " sec.)" << endl;
}


int FFA(Graphe const&g, int s, int t, vector<bool> &ret) //ret vaut true si le sommet est avec i
{
	vector<vector<int>> c; //matrice des capacités
	vector<int> pred; //prédécesseur dans le chemin augmentant
	vector<bool> mark;
	vector<int> q;
	int val = 0;
	q.resize(g.n);
	mark.resize(g.n);
	pred.resize(g.n);
	mark.resize(g.n);
	c.resize(g.n);
	for (int i = 0; i < g.n; i++)
		c[i].resize(g.n);
	for (int i = 0; i < g.n; i++)
		for (auto j = g.voisins[i].begin(); j != g.voisins[i].end(); j++)
			c[i][*j] = 1;
	bool augmented = true;
	while (augmented)
	{
		augmented = false;
		int rd = 0, wr = 0;
		for (int i = 0; i < g.n; i++)
			mark[i] = false;
		fill(pred.begin(), pred.end(), t);
		q[wr++] = s;
		mark[s] = true;
		while (rd < wr)
		{
			int k = q[rd++];
			for(auto m=g.voisins[k].begin(); m!=g.voisins[k].end(); m++)
				if (!mark[*m] && c[k][*m]>0)
				{
					if (*m == t)
					{
						int i=k,j=*m;
						while (i != t)
						{
							c[i][j]--;
							c[j][i]++;
							j = i;
							i = pred[i];
							augmented = true;
						}
						val++;
						rd = g.n+1;
						break;
					}
					pred[*m] = k;
					mark[*m] = true;
					q[wr++] = *m;
				}
		}
	}
	//on a obtenu un flot max, on va en déduire la partition correspondant à la coupe min avec un BFS
	ret.resize(g.n);
	int wr = 0, rd = 0;
	for (int i = 0; i < g.n; i++)
		ret[i] = false;
	q[wr++] = s;
	ret[s] = true;
	while (rd < wr)
	{
		int i = q[rd++];
		for(auto m=g.voisins[i].begin(); m!=g.voisins[i].end(); m++)
			if (!ret[*m] && c[i][*m] > 0)
			{
				q[wr++] = *m;
				ret[*m] = true;
			}
	}
	return val;
}

