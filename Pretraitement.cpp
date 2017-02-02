#include "Pretraitement.h"
#include <list>
#include <iostream>
#include <time.h>
#include <fstream>
using namespace std;

bool pretraitement(Graphe const & g, Graphe const & _g, vector<vector<bool>> & x_mask)
{
	clock_t t0 = clock();
	cout << "Debut du pretraitement ... ";
	int N = g.n, _N = _g.n;
	list<unsigned int>::iterator l, _l;
	if (N < _N)
		return true;
	x_mask.resize(_N);
	for (int i = 0; i < _N; i++)
		x_mask[i].resize(N);
	for (int i = 0; i < _N; i++)
		for (int j = 0; j < N; j++)
			x_mask[i][j] = true;
	vector<list<unsigned int>> a_ordered;
	a_ordered.resize(_N + N);
	//on trie par ligne les coefficients de D
	for (int m = 0; m < _N + N; m++)
		a_ordered[m].clear();
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			a_ordered[i].push_back(g.D[i][j]);
			if (i < _N && j < _N)
				a_ordered[N + i].push_back(_g.D[i][j]);
			else if (i < _N)
				a_ordered[N + i].push_back(0);
		}
	for (int m = 0; m < _N + N; m++)
		a_ordered[m].sort();
	//on vérifie si il y a des incompatibilités.
	for (int i = 0; i < _N; i++)
		for (int j = 0; j < N; j++)
			for (l = a_ordered[j].begin(), _l = a_ordered[i + N].begin(); l != a_ordered[j].end(), _l != a_ordered[i + N].end(); l++, _l++)
				if (*l < *_l)
				{
					x_mask[i][j] = false;
					break;
				}
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
	int n, _n,s;
	clock_t t0 = clock();
	if (!f)
		return false;
	cout << "Chargement du pretraitement ... ";
	f >> _n;
	x_mask.resize(_n);
	f >> n;
	for (int i = 0; i < _n; i++)
		x_mask[i].resize(n);
	for(int i=0; i<_n; i++)
		for (int j = 0; j < n; j++)
		{
			f >> s;
			x_mask[i][j] = s == 1;
		}
	cout << "OK" << endl;
}

