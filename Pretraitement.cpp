#include "Pretraitement.h"
#include <list>
#include <iostream>
using namespace std;

bool pretraitement(Graphe const & g, Graphe const & _g,vector<vector<bool>> & x_mask)
{
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
			
	/*bool changed=true;
	int one_index;
	while(changed)
	{
		changed=false;
		for(int i=0; i<N; i++)
		{
			one_index=-1;
			for(int j=0; j<N; j++)
			{
				if(x_mask[i][j])
				{
					if(one_index!=-1)
					{
						one_index=-2;
						break;
					}
					one_index=j;
				}
			}
			if(one_index==-1)
				return true;
			if(one_index!=-2)
				for(int j=0; j<N; j++)
					if(j!=i)
					{
						if(x_mask[j][one_index])
							changed=true;
						x_mask[j][one_index]=false;
					}
			one_index=-1;
			for(int j=0; j<N; j++)
			{
				if(x_mask[j][i])
				{
					if(one_index!=-1)
					{
						one_index=-2;
						break;
					}
					one_index=j;
				}
			}
			if(one_index==-1)
				return true;
			if(one_index!=-2)
				for(int j=0; j<N; j++)
					if(j!=i)
					{
						if(x_mask[one_index][j])
							changed=true;
						x_mask[one_index][j]=false;
					}
		}
	}*/
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

void loadModel(IloEnv env, IloModel model, Graphe const & g, Graphe const & _g,vector<vector<bool>> const & x_mask,IloBoolVarArray x)
{
	int offset=0; //||A||-||A_barre|| (mais elles ne contiennent que des coefficients positifs)
	int N=g.n,_N=_g.n;
	/*for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
		{
			if(i<_N && j<_N && _g.A[i].test(j))
				offset-=1;
			if(g.A[i].test(j))
			offset+=1;
		}

	IloExpr objectif(env);
	cout<< "N="<<N<<endl;
	for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
			for(int k=0; k<N; k++)
			{
				if(x_mask[i][k] && g.A[k].test(j))
				objectif+=x[i*N+k];
				if(i<_N && k<_N && x_mask[k][j] && _g.A[i].test(k))
				objectif-=x[k*N+j];
			}
	cout<<"offset="<<offset<<endl;*/
	model.add(IloMinimize(env,0));

	//contrainte sur la somme des x sur les ligne et collonnes
	IloExprArray ligne(env,_N);
	for(int m=0; m<_N; m++)
		ligne[m]=IloExpr(env);
	IloExprArray collonne(env,N);
	for(int m=0; m<N; m++)
		collonne[m]=IloExpr(env);


	for(int i=0; i<N; i++)
	{
		for(int j=0; j<N; j++)
		{
			if(x_mask[i][j] && i<_N)
			ligne[i]+=x[i*N+j];
			if(x_mask[j][i] && j<_N)
			collonne[i]+=x[j*N+i];
			if(!x_mask[i][j] || i>=_N)
				model.add(x[i*N+j]==0); //contrainte liées au prétraitement
		}
		//contraintes de X matrice de transposition, contrainte paire sur les colonnes et contraintes impaires sur les lignes (je crois)
		if(i<_N)
		model.add(ligne[i]==1);
		model.add(collonne[i]<=1);
	}
	IloExprArray y_expr(env,N*_g.n);
	for(int m=0; m<_g.n*N; m++)
		y_expr[m]=IloExpr(env);
	for(int i=0; i<_N; i++)
		for(int j=0; j<N; j++)
		{
			for(int k=0; k<N; k++)
			{
				if(x_mask[i][k] && g.A[k].test(j))
				y_expr[i*N+j]+=x[k+N*i];
				if(k<_N && x_mask[k][j] && _g.A[i].test(k))
				y_expr[i*N+j]-=x[k*N+j];
			}
			model.add(y_expr[i*N+j]>=0);
		}
}