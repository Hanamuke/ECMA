/*
Algorithme en O(MN^2) (avec M la cardinalit� de la plus petite des deux parties) pour trouver un couplage maximal dans un graphe biparti.
Impl�mentation adapt�e de celle disponible ici (pour le cas matrice non carr�e), pour plus de d�tails
https://www.topcoder.com/community/data-science/data-science-tutorials/assignment-problem-and-hungarian-algorithm/
*/
#include "Khun.h"
#include <algorithm>
#include <iostream>

int N, M, best_match; //dimensions de la matrice, cardinalit� du couplage courant ( c'est un algo dual, il est toujours dual r�alisable )
vector<vector<int>> profit;
vector<int> labelX, labelY; //variables duales du probl�me
vector<int> XY, YX; //sommet match� avec X et Y respectivement 
vector<bool> S, T; //Sous-ensembles de X et Y respectivement. 
vector<int> slack, slackX; //enregistre de combien on pourra ajuster les labels par ajout d'ar�tes au matching
vector<int> previous;//enregistre le parcours du chemin augmentant.

void augment();
void update_labels();
void add_to_tree(int x, int prevx);

int khun(vector<vector<int>> const & in, vector<int>& ret)
{
	//Attention, la premi�re dimension de profit doit �tre la plus petite 
	int val = 0;//poids du meilleur couplage trouv�
	bool swap;
	N = in.size();
	M = in[0].size();
	if (M < N)
	{
		swap = true;
		profit.resize(M);
		for (int i = 0; i < M; i++)
			profit[i].resize(N);
		for (int y = 0; y < N; y++)
			for (int x = 0; x < M; x++)
				profit[x][y] = in[y][x];
		int tmp = N;
		N = M;
		M = tmp;
	}
	else
	{
		swap = false;
		profit.resize(N);
		for (int i = 0; i < N; i++)
			profit[i].resize(M);
		for (int y = 0; y < M; y++)
			for (int x = 0; x < N; x++)
				profit[x][y] = in[x][y];
	}
	//ici on a toujours M>=N
	initialize(in);
	augment();
	for (int x = 0; x < N; x++)
		val += profit[x][XY[x]];
	if (swap)
		ret = YX;
	else
		ret = XY;
	return val;
}

void initialize(vector<vector<int>> const & in)
{
	best_match = 0;
	labelX.resize(N);
	fill(labelX.begin(), labelX.end(), 0);
	for (int x = 0; x < N; x++)
		for (int y = 0; y < M; y++)
			labelX[x] = labelX[x] > profit[x][y] ? labelX[x] : profit[x][y];
	labelY.resize(M);
	fill(labelY.begin(), labelY.end(), 0);
	XY.resize(N);
	fill(XY.begin(), XY.end(), -1);
	YX.resize(M);
	fill(YX.begin(), YX.end(), -1);
	T.resize(M);
	S.resize(N);
	slack.resize(M);
	slackX.resize(M);
	previous.resize(N);
}

void augment()
{
	if (best_match == N)
		return;
	int x, y, root;
	int wr = 0, rd = 0;//positions de lecture et d'�criture dans la queue on �crit avec q[wr++] et on lit avec q[rd++]
	vector<int> q; //queue pour le BFS
	q.resize(M);
	fill(S.begin(), S.end(), false);
	fill(T.begin(), T.end(), false);
	fill(previous.begin(), previous.end(), -1);

	for (x = 0; x < N; x++) //on prend pour racine de notre cha�ne augmentante le premier sommet qui n'est pas macth�
		if (XY[x] == -1)
		{
			q[wr++] = root = x;
			previous[x] = -2;
			S[x] = true;
			break;
		}

	for (y = 0; y < M; y++)//on initialise le tableau de slack;
	{
		slack[y] = labelX[root] + labelY[y] - profit[root][y];//voir formulation duale du probl�me.
		slackX[y] = root;
	}

	while (true)
	{
		while (rd < wr) //tant qu'on a pas fini le BFS
		{
			x = q[rd++];//on parcours le prochain sommet
			for (y = 0; y < M; y++)
				if (profit[x][y] == labelX[x] + labelY[y] && !T[y])//si une arr�te est elligible pour �tre ajout�e au matching dont le sommet dans Y n'est pas d�j� dans la cha�ne
				{
					if (YX[y] == -1) //si y n'est pas d�j� match�, on peut simplement ajouter l'ar�te
						break;
					T[y] = true; //sinon on ajoute y � la cha�ne et on continue le BFS
					q[wr++] = YX[y]; //on ajoute y au BFS
					add_to_tree(YX[y], x);//on ajoute l'ar�te � la cha�ne augmentante
				}
			if (y < M)
				break;
		}
		if (y < M)
			break;
		update_labels();
		wr = rd = 0;
		for (y = 0; y < M; y++)
			if (!T[y] && slack[y] == 0)
			{
				if (YX[y] == -1)
				{
					x = slackX[y];
					break;
				}
				else
				{
					T[y] = true;
					if (!S[YX[y]])
					{
						q[wr++] = YX[y];
						add_to_tree(YX[y], slackX[y]);
					}
				}
			}
		if (y < M)
			break;
	}
	if (y < M)
	{
		best_match++;
		for (int cx = x, cy = y, ty;cx != -2; cx = previous[cx], cy = ty) //on remonte la cha�ne augmentante trouv�e en inversant le couplage
		{
			ty = XY[cx];
			YX[cy] = cx;
			XY[cx] = cy;
		}
		augment();
	}
	return;
}


void update_labels() //O(N)
{
	int x, y, delta = INT_MAX;
	for (y = 0; y < M; y++)
		if (!T[y])
			delta = delta < slack[y] ? delta : slack[y];//calcul du changement des labels
	for (x = 0; x < N; x++)
		if (S[x])
			labelX[x] -= delta;//mise � jour de labelX
	for (y = 0; y < M; y++)
		if (T[y])
			labelY[y] += delta;
		else
			slack[y] -= delta; //update du slack;
	return;
}

void add_to_tree(int x, int prevx) // ajoute x � S , O(n), prevx �tant le sommet de S pr�c�dent dans la cha�ne augmentante.
{
	S[x] = true;
	previous[x] = prevx;
	for (int y = 0; y < M; y++) //comme on a ajout� un sommet � S, on doit ajuster les slacks.
		if (labelX[x] + labelY[y] - profit[x][y] < slack[y])
		{
			slack[y] = labelX[x] + labelY[y] - profit[x][y];
			slackX[y] = x;
		}
}
