#include "Hopcroft.h"
#include <iostream>
#include <algorithm>

namespace Algo
{
	int N, M; //dimensions du graphe
	vector<int> YX; //sommet matché respectivement avec X et Y
	vector<int> dist;//distance des sommets de X à un sommet non couplé de X
	vector<int> q;//queue pour le BFS
	vector<int> randN;
	vector<int> randM;
	int NIL; //sommet nul;

	bool BFS(vector<vector<bool>> const & in, vector<int> & XY);
	bool DFS(int x, vector<vector<bool>> const & in, vector<int> & XY);

	int Hopcroft(vector<vector<bool>> const & in, vector<int> &XY)
	{
		N = in.size();
		M = in[0].size();
		NIL = N + M;
		XY.resize(N);
		YX.resize(M);
		dist.resize(N + M + 1);
		fill(XY.begin(), XY.end(), NIL);
		fill(YX.begin(), YX.end(), NIL);
		q.resize(N);
		randN.resize(N);
		randM.resize(M);
		for (int i = 0; i < M; i++)
		{
			randM[i] = i;
			if (i < N)
				randN[i] = i;
		}
		random_shuffle(randN.begin(), randN.end());
		random_shuffle(randM.begin(), randM.end());
		int ret = 0;//cardinality du couplage actuel
		while ((ret < M && ret < N) && BFS(in, XY)) //tant que le couplage n'est pas maximal et qu'on trouve des chemins augmentant
			for (auto x = randN.begin(); x != randN.end(); x++)
				if (XY[*x] == NIL)
					if (DFS(*x, in, XY))
						ret++;
		return ret;
	}

	bool BFS(vector<vector<bool>> const & in, vector<int> & XY)
	{
		int wr = 0, rd = 0;
		for (auto x = randN.begin(); x != randN.end(); x++) //pour tout sommet de X
			if (XY[*x] == NIL)//si x n'est pas dans le couplage
			{
				dist[*x] = 0;
				q[wr++] = *x; //on prend x comme point un des départ du BFS
			}
			else
				dist[*x] = INT_MAX;//initialisation
		dist[NIL] = INT_MAX; // distance du puits aux sommets non couplés
		while (rd < wr)//tant que le BFS n'est  pas terminé
		{
			int x = q[rd++];
			if (dist[x] < dist[NIL])
				for (auto y = randM.begin(); y != randM.end(); y++)
					if (in[x][*y] && dist[YX[*y]] == INT_MAX)//on met à jour les distance et on ajoute les sommets au BFS
					{
						dist[YX[*y]] = dist[x] + 1; //YX est initialisé à NIL, ainsi dist[NIL] peut etre actualisé ici.
						if (YX[*y] != NIL)
							q[wr++] = YX[*y];
					}
		}
		return dist[NIL] != INT_MAX; //si on a trouvé un chemin augmentant
	}

	bool DFS(int x, vector<vector<bool>> const & in, vector<int> & XY)
	{
		if (x != NIL)//
		{
			for (auto y = randM.begin(); y != randM.end(); y++)

				if (in[x][*y] && dist[YX[*y]] == dist[x] + 1)
					/*il se peut que dist[YX[y]]=0 le successeur de x dans la chaîne au changé depuis que lon a relevé la chaîne
					si on déjà retourné des portions de la chaîne augmentante,
					et qu'ainsi un sommet ayant été identifié comme étant part d'une chaîne augmentant ne soit finalement pas couplé
					, on retourne alors faux et on marque le sommet en mettant une distance infinie pour stopper
					les prochains parcours en profondeur le rencontrant */
					if (DFS(YX[*y], in, XY))//devrait toujours être vrai, on inverse la suite de la chaîne, puis la portion en cours
					{
						YX[*y] = x;
						XY[x] = *y;
						return true;
					}
			dist[x] = INT_MAX;//pour eviter de reparcourir x s'il n'est pas le début d'une chaîne augmentante.
			return false;
		}
		return true;
	}
}