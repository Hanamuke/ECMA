#include "Pretraitement.h"
#include <list>
#include <iostream>
#include <time.h>
using namespace std;

bool pretraitement(Graphe const & g, Graphe const & _g,vector<vector<bool>> & x_mask)
{
	clock_t t0=clock();
	cout<<"Debut du pretraitement ... ";
	int N=g.n,_N=_g.n;
	list<unsigned int>::iterator l,_l;
	if(N<_N)
		return true;
	x_mask.resize(N);
	for(int i=0; i<N; i++)
		x_mask[i].resize(N);
	for(int i=0; i<N; i++)
	for(int j=0; j<N; j++)
	x_mask[i][j]=true;
	vector<list<unsigned int>> a_ordered; 
	a_ordered.resize(2*N);
	//on trie par ligne les coefficients de D
	for(int m=0; m<2*N; m++)
		a_ordered[m].clear();
	for(int i=0; i<N; i++)
	for(int j=0; j<N; j++)
	{
		a_ordered[2*i].push_back(g.D[i][j]);
		if(i<_N && j<_N)
			a_ordered[2*i+1].push_back(_g.D[i][j]);
		else
			a_ordered[2*i+1].push_back(0);
	}
	for(int m=0; m<2*N; m++)
		a_ordered[m].sort();
	//on vérifie si il y a des incompatibilités.
	for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
			for(l=a_ordered[2*j].begin(),_l=a_ordered[2*i+1].begin(); l!=a_ordered[2*j].end(),_l!=a_ordered[2*i+1].end(); l++,_l++)
				if(*l<*_l)
				{
					x_mask[i][j]=false;
					break;
				}
	cout<<"OK ("<<(double)(clock()-t0)/(double)CLOCKS_PER_SEC<<".sec)"<<endl;
	return false;
}

void print(vector<vector<bool>> & x_mask)
{
	int cnt=0;
	for(auto i=x_mask.begin(); i!=x_mask.end(); i++)
	{
		for(auto j=i->begin(); j!=i->end(); j++)
		{
			cout<<(int)*j<<" ";
			if(*j)
				cnt++;
		}	
		cout<<endl;
	}
	cout<<"non-zero : "<<cnt<<"/"<<x_mask.size()*x_mask.size()<<endl;
}
