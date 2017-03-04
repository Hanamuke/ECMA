#include "Pretraitement.h"
#include "Hopcroft.h"
#include <list>
#include <iostream>
#include <time.h>
#include <fstream>
#include <queue>
#include <set>
using namespace std;
void setArc(Graphe const &, Graphe const &, vector<vector<bool>>&, int, int, vector<int> &, int&);
void unsetArc(vector<vector<bool>>&, vector<int> &, int&);
void AC(Graphe const &, Graphe const &, vector<vector<bool>>&, vector<vector<int>>&);
void SC(Graphe const &, Graphe const &, vector<vector<bool>>&);
bool pretraitement(Graphe const & g, Graphe const & _g, vector<vector<bool>> & x_mask)
{
	vector<int> f_mask;
	f_mask.resize(g.n*_g.n);
	int ptr = 0;
	clock_t t0 = clock();
	cout << "Debut du pretraitement ... ";
	int N = g.n, _N = _g.n;
	vector<vector<int>> nbCouplage; //nombre de couplage donc [i][j] est membre, sans compter son propre couplage
	if (N < _N)
		return true;
	x_mask.resize(_N);
	nbCouplage.resize(_N);

	for (int i = 0; i < _N; i++)
	{
		x_mask[i].resize(N);
		nbCouplage[i].resize(N);
	}

	for (int i = 0; i < _N; i++)
		for (int j = 0; j < N; j++)
			x_mask[i][j] = true;

	SC(g, _g, x_mask);
	AC(g, _g, x_mask, nbCouplage); //j'ai randomisé l'algo de couplage 
	//j'ai l'impression que ça l'a ralenti un peu mais nbCouplage est équilibré avant ça chargait la diagonale.
	/*for (int j = 0; j < N; j++)
			cout << nbCouplage[0][j] <<" ";
	cout << endl;*/
	/************************************
	Exemple pour l'heuristique:
	setArc(g, _g, x_mask, 0, 0, f_mask,ptr);
	if(x_mask[1][1]) //on peut en forcer plusieurs mais ATTENTION si [1,1] était forcé à false
	//en forçant [0,0] à true, on cours àla catastrophe.
	setArc(g, _g, x_mask, 1, 1, f_mask,ptr);
	vector<vector<bool>> essai=x_mask;
	AC(g,_g,essai) //essai de propagation sur une copie
	if(essai doesn't work) // eventuellement à tester avec cplex.
	unsetArc(x_mask, f_mask, ptr); //on annule
	************************************/
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

void setArc(Graphe const & g, Graphe const & _g, vector<vector<bool>>&in, int i, int j, vector<int> & mask_f, int &ptr) //O(N*_N)
{
	for (int m = 0; m < _g.n; m++)
		if (m != i && in[m][j])
		{
			in[m][j] = false;
			mask_f[ptr++] = m*g.n + j;
		}
	for (int m = 0; m < g.n; m++)
		if (m != j && in[i][m])
		{
			in[i][m] = false;
			mask_f[ptr++] = i*g.n + m;
		}
	//on ajoute les contraintes d'exclusion liées à xij=1
	for (int l = 0; l < _g.n; l++)
		for (int m = 0; m < g.n; m++)
			if (in[l][m] && (_g.D[i][l] < g.D[j][m] || _g.C[i][l] > g.C[j][m]))
			{
				in[l][m] = false;
				mask_f[ptr++] = l*g.n + m;
			}
}

void unsetArc(vector<vector<bool>>& in, vector<int> & mask_f, int& ptr)
{
	int N = in[0].size();
	while (ptr > 0)
	{
		//j'aurais aimé faire ça en une seule ligne mais je crois qu'avec deux décrément dans la même ligne ça le décrémente deux fois, en tout cas ça marche pas.
		--ptr;
		in[mask_f[ptr] / N][mask_f[ptr] % N] = true;
	}
}

void AC(Graphe const & g, Graphe const & _g, vector<vector<bool>>& in, vector<vector<int>>& nbCouplage)
{
	//[i,j] -> i*N+j 
	int N = g.n, _N = _g.n;
	vector<set<int>> dependencies; //ensemble des arcs qui dépendent de l'arc  -> O(n^3) en mémoire, 
	//ce serait n^4 avec de simples booléens, mais on gagnerait très légèrement en vitesse.
	vector<vector<int>> XY; //couplage obtenu par l'algorithme de Hopcroft-Karp
	queue<int> q; //liste des arcs à vérifier, on utilise pas vector comme ailleurs dans le programme comme on va vérifier plusieurs fois
	//des arcs, on ne sait pas quel taille il va falloir
	vector<bool> inQueue; //indiquateur des arcs dans la queue.
	vector<int> mask_f; //enregistre les changements pour make/unmake
	int ptr = 0; //nombre d'éléments dans mask_f
	mask_f.resize(N*_N);
	dependencies.resize(N*_N);
	inQueue.resize(N*_N);
	XY.resize(N*_N);
	for (int i = 0; i < _N*N; i++)
		XY[i].resize(_N);
	for (int i = 0; i < _N; i++) //on met tous les arcs à 1 dans la queue
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
		setArc(g, _g, in, i, j, mask_f, ptr); //on force l'arc à 1 et on propage les contraintes
		for (int l = 0; l < _N; l++)
			dependencies[XY[index][l] + N*l].erase(index); //on supprime les anciennes dépendances de [i;j]
		if (Algo::Hopcroft(in, XY[index]) < _N) //si on ne trouve pas de couplage
		{
			/********************************
			Seul endroit ou l'entrée in est réelement modifier (sans annuler le changement).
			Si on veut enregistrer ce qui a changé pour annuler quelque chose c'est ici qu'il faut y ajouter.
			********************************/
			in[i][j] = false; //on force l'arc à 0;
			for (auto k = dependencies[index].begin(); k != dependencies[index].end(); k++)//il faut alors vérifier les arcs qui en étaient dépendant
				if (!inQueue[*k])
				{
					q.push(*k);
					inQueue[*k] = true;
				}
		}
		else //sinon on notes les nouvelles dépendances de [i;j]
		{
			for (int l = 0; l != _N; l++)
				if (l != i)
					dependencies[XY[index][l] + N*l].insert(index);
		}
		unsetArc(in, mask_f, ptr); //on annule les changements du au forcage de l'arc à 1.
	}
	for (int i = 0; i < _N; i++)
		for (int j = 0; j < N; j++)
			nbCouplage[i][j] = dependencies[i*N + j].size();
}

void SC(Graphe const&g, Graphe const&_g, vector<vector<bool>> &in)
{
	for (int i = 0; i < _g.n; i++)
		for (int j = 0; j < g.n; j++)
			if (g.degre[j] < _g.degre[i] || g.cycle_min[j] > _g.cycle_min[i])
				in[i][j] = 0;
}
