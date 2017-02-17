#include "Pretraitement.h"
#include "Hopcroft.h"
#include <list>
#include <iostream>
#include <time.h>
#include <fstream>
#include <queue>
#include <set>
using namespace std;
int setArc(Graphe const &, Graphe const &, vector<vector<bool>>&, int, int, vector<int> &);
void AC(Graphe const &, Graphe const &, vector<vector<bool>>&);
void SC(Graphe const &, Graphe const &, vector<vector<bool>>&);
bool pretraitement(Graphe const & g, Graphe const & _g, vector<vector<bool>> & x_mask)
{
	clock_t t0 = clock();
	cout << "Debut du pretraitement ... ";
	int N = g.n, _N = _g.n;
	if (N < _N)
		return true;
	x_mask.resize(_N);
	for (int i = 0; i < _N; i++)
		x_mask[i].resize(N);

	for (int i = 0; i < _N; i++)
		for (int j = 0; j < N; j++)
			x_mask[i][j] = true;
	SC(g, _g, x_mask);
	AC(g, _g, x_mask);
	cout << "OK (" << (double)(clock() - t0) / (double)CLOCKS_PER_SEC << ".sec)" << endl;
	return false;
}

void save_pretraitement(string path, vector<vector<bool>> const & x_mask)
{
	fstream out(path, fstream::out | fstream::trunc);
	int n = x_mask.size(), _n, cnt = 0;
	if (n == 0)
		return;
	_n = x_mask[0].size();
	out << n << endl;
	out << _n << endl;
	for (auto i = x_mask.begin(); i != x_mask.end(); i++)
	{
		for (auto j = i->begin(); j != i->end(); j++)
			if (*j)
				out << "1 ";
			else
			{
				out << "0 ";
				cnt++;
			}
		out << endl;
	}
	out << "Le pretraitement a fixe " << cnt << " variables a 0, soit " << (double)(cnt * 100) / (double)(n*_n) << "% des variables.";
	cout << "Pretraitement sauvegarde." << endl;
	return;
}

bool load_pretraitement(string path, vector<vector<bool>> & x_mask)
{
	fstream f(path, fstream::in);
	int n, _n, s;
	clock_t t0 = clock();
	if (!f)
		return false;
	cout << "Chargement du pretraitement ... ";
	f >> _n;
	x_mask.resize(_n);
	f >> n;
	for (int i = 0; i < _n; i++)
		x_mask[i].resize(n);
	for (int i = 0; i < _n; i++)
		for (int j = 0; j < n; j++)
		{
			f >> s;
			x_mask[i][j] = s == 1;
		}
	cout << "OK" << endl;
}

int setArc(Graphe const & g, Graphe const & _g, vector<vector<bool>>&in, int i, int j, vector<int> & mask_f) //O(N*_N)
{
	int ret = 0;
	for (int m = 0; m < _g.n; m++)
		if (m != i && in[m][j])
		{
			in[m][j] = false;
			mask_f[ret++] = m*g.n + j;
		}
	for (int m = 0; m < g.n; m++)
		if (m != j && in[i][m])
		{
			in[i][m] = false;
			mask_f[ret++] = i*g.n + m;
		}
	//on ajoute les contraintes d'exclusion liées à xij=1
	for (int l = 0; l < _g.n; l++)
		for (int m = 0; m < g.n; m++)
			if (in[l][m] && _g.D[i][l] < g.D[j][m])
			{
				in[l][m] = false;
				mask_f[ret++] = l*g.n + m;
			}
	return ret;
}

void AC(Graphe const & g, Graphe const & _g, vector<vector<bool>>& in)
{
	//[i,j] -> i*N+j 
	int N = g.n, _N = _g.n;
	vector<set<int>> dependencies; //liste des arcs qui dépendent de l'arc
	vector<vector<int>> XY; //couplage obtenu pas l'algorithme de Hopcroft-Karp
	queue<int> q; //liste des arcs à vérifier
	vector<vector<bool>> rmask = in;
	vector<bool> inQueue;
	vector<int> mask_f;
	int ptr = 0;
	int changei = -1, changej = -1;
	mask_f.resize(N*_N);
	dependencies.resize(N*_N);
	inQueue.resize(N*_N);
	XY.resize(N*_N);
	for (int i = 0; i < _N*N; i++)
		XY[i].resize(_N);
	for (int i = 0; i < _N; i++)
		for (int j = 0; j < N; j++)
			if (in[i][j])
			{
				q.push(i*N + j);
				inQueue[i*N + j] = true;
			}
			else
				inQueue[i*N + j] = false;
	while (!q.empty())
	{
		int i = q.front() / N, j = q.front() % N, index = q.front();
		q.pop();
		inQueue[index] = false;
		while (ptr > 0)
		{
			//j'aurais aimé faire ça en une seule ligne mais je crois qu'avec deux décrément dans la même ligne ça le décrémente deux fois, en tout cas ça marche pas.
			--ptr;
			rmask[mask_f[ptr] / N][mask_f[ptr] % N] = true;
		}
		if (changei != -1)
			rmask[changei][changej] = false;
		ptr = setArc(g, _g, rmask, i, j, mask_f);
		for (int l = 0; l < _N; l++)
			dependencies[XY[index][l] + N*l].erase(index); //on supprime les anciennes dépendances de [i;j]
		if (Algo::Hopcroft(rmask, XY[index]) < _N) //si on ne trouve pas de couplage
		{
			in[i][j] = 0; //on force l'arc à 0;
			changei = i;
			changej = j;
			for (auto k = dependencies[index].begin(); k != dependencies[index].end(); k++)//il faut alors vérifier les arcs qui en étaient dépendant
				if (!inQueue[*k])
				{
					q.push(*k);
					inQueue[*k] = true;
				}
		}
		else //sinon on notes les nouvelles dépendances de [i;j]
		{
			changei = -1;
			for (int l = 0; l != _N; l++)
				if (l != i)
					dependencies[XY[index][l] + N*l].insert(index);
		}
	}
}

void SC(Graphe const&g, Graphe const&_g, vector<vector<bool>> &in)
{
	for (int i = 0; i < _g.n; i++)
		for (int j = 0; j < g.n; j++)
			if (g.degre[j] < _g.degre[i])
				in[i][j] = 0;
	return;
}
