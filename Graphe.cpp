#include "Graphe.h"
#include <fstream>
#include <iostream>
using namespace std;

void end(Graphe & g)
{
	if(g.A != NULL)
		delete[]g.A;
	g.A=NULL;
}
void init(Graphe & g, string filename)
{
	if(g.A!=NULL)
		delete[]g.A;
	fstream f(filename,fstream::in);
	int k,i,j;
	if(!f)
	{
		cout<<"Fichier introuvable"<<endl;
		g.A=NULL;
		g.n=0;
		return;
	}
	//on lit n et on alloue la mémoire.
	f>>g.n;
	if(g.n<=0)
	{
		cout<<"Fichier invalide"<<endl;
		g.A=NULL;
		return;
	}
	g.A=new uint32_t[g.n*g.n*g.n];
	g.a_liste.resize(g.n);
	for(i=0; i<g.n*g.n*g.n; i++)
		g.A[i]=0;
	//on rempli la matrice d'adjacence
	for(i=0; i<g.n; i++)
	{
		int k;
		f>>k;
		g.a_liste[i].resize(k);
		for(int m=0; m<k; m++)
		{
			f>>j;
			g.A[g.n*i+j]=1;
			g.a_liste[i][m]=j;
		}
	}
	uint32_t *left=g.A+g.n*g.n,*right=g.A;
	for(k=1; k<g.n-1; k++)
	for(i=0; i<g.n; i++)
	{
		for(j=0; j<g.n; j++)
		{
			for(auto m=g.a_liste[j].begin();m!=g.a_liste[j].end();m++)
			{
				if(0xffffffffULL-(*(right+*m))<*left)
				{
					g.k_max=k-1;
					cout<<"k_max = "<<g.k_max<<endl;
					return;
				}
				*left+=(*(right+*m));
			}
			left++;
		}
		right+=g.n;
	}
	
	right=g.A;
	//calcul de A*
	for(k=0; k<g.n-1; k++)
	{
		left=g.A+(g.n-1)*g.n*g.n;
		for(i=0; i<g.n; i++)
		for(j=0; j<g.n; j++)
		{
			if(0xffffffffULL-*right<*left)
			{
				g.k_max=g.n-2;
				cout<<"k_max = "<<g.k_max<<endl;
				return;
			}
			*left+=*right;
			right++;
			left++;
		}
	}
	g.k_max=g.n-1;
	cout<<"k_max = "<<g.k_max<<endl;	
}

void print(Graphe const & g, int k)
{
	if(k>g.k_max)
		k=g.k_max;
	for(int i=0; i<g.n; i++)
	{
		for(int j=0; j<g.n; j++)
			cout<<g.A[k*g.n*g.n+i*g.n+j]<<" ";
		cout<<endl;
	}
}
