#include "Graphe.h"
#include <fstream>
#include <iostream>
using namespace std;

void init(Graphe & g, string filename)
{
	vector<bitset<NMAX>> An,temp;
	fstream f(filename,fstream::in);
	int k,i,j,n=0;
	if(!f)
	{
		cout<<"Fichier introuvable"<<endl;
		g.n=0;
		return;
	}
	//on lit n et on alloue la mémoire.
	f>>n;
	g.n=n;
	if(n<=0)
	{
		cout<<"Fichier invalide"<<endl;
		return;
	}
	g.A.resize(n);
	g.D.resize(n);
	temp.resize(n);
	An.resize(n);
	for(i=0; i<n; i++)
	{
		g.A[i].reset();
		g.D[i].resize(n);
		for(j=0; j<n; j++)
			g.D[i][j]=NMAX;

	}
	//on rempli la matrice d'adjacence
	for(i=0; i<n; i++)
	{
		int k;
		f>>k;
		for(int m=0; m<k; m++)
		{
			f>>j;
			g.A[i].set(j);
		}
	}
	for(i=0; i<n; i++)
	{
		g.D[i][i]=0;
		for(j=0; j<n; j++)
			if(g.A[i].test(j))
				g.D[i][j]=1;
	}
	An=g.A;
	for(k=2; k<n; k++)
	{
		//calcul de A*An
		for(i=0; i<n; i++)
		for(j=0; j<n; j++)
		temp[i].set(j,(An[i]&g.A[j]).any());
		An=temp;
		//mise à jour de D
		for(i=0; i<n; i++)
		for(j=0; j<n; j++)
			if(g.D[i][j]==NMAX && An[i].test(j))
				g.D[i][j]=k;
	}
	for(i=0; i<n; i++)
		for(j=0; j<n; j++)
			g.D[i][j]=NMAX-g.D[i][j];
}

