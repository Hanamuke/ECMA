#include "Pretraitement.h"
#include <list>
#include <iostream>
using namespace std;

bool pretraitement(Graphe const & g, Graphe const & _g,vector<vector<bool>> & x_mask)
{
	int N=g.n,_N=_g.n , K=g.k_max>=_g.k_max?_g.k_max:g.k_max;
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
	for(int k=0; k<K; k++)
	{
		for(int m=0; m<2*N; m++)
			a_ordered[m].clear();
		for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
		{
			a_ordered[2*i].push_back(g.A[k*N*N+i*N+j]);
			if(i<_N && j<_N)
				a_ordered[2*i+1].push_back(_g.A[k*_N*_N+i*_N+j]);
			else
				a_ordered[2*i+1].push_back(0);
		}
		for(int m=0; m<2*N; m++)
			a_ordered[m].sort();
		for(int i=0; i<N; i++)
			for(int j=0; j<N; j++)
				if(x_mask[i][j])
				for(l=a_ordered[2*i].begin(),_l=a_ordered[2*j+1].begin(); l!=a_ordered[2*i].end(),_l!=a_ordered[2*j+1].end(); l++,_l++)
					if(*l<*_l)
					{
						x_mask[j][i]=false;
						break;
					}
	}
	bool changed=true;
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
	}
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
	int N=g.n;
	for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
		{
			if(i<_g.n && j<_g.n)
				offset-=_g.A[i*_g.n+j];
			offset+=g.A[i*N+j];
		}

	IloExpr expr(env);
	cout<< "N="<<N<<endl;
	for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
			for(int k=0; k<N; k++)
			{
				expr+=x[i*N+k]*(int)g.A[k*N+j];
				if(i<_g.n && k<_g.n)
				expr-=(int)_g.A[i*_g.n+k]*x[k*N+j];
			}
	cout<<"offset="<<offset<<endl;
	model.add(IloMinimize(env,expr));
	model.add(expr==offset);
	IloExprArray x_expr(env,2*N);
	for(int m=0; m<2*N; m++)
		x_expr[m]=IloExpr(env);
	for(int i=0; i<N; i++)
	{
		for(int j=0; j<N; j++)
		{
			x_expr[2*i]+=x[i+N*j];
			x_expr[2*i+1]+=x[j+N*i];
			if(!x_mask[i][j])
				model.add(x[i*N+j]==0); //contrainte liées au prétraitement
		}
		//contraintes de X matrice de transposition, contrainte paire sur les colonnes et contraintes impaires sur les lignes (je crois)
		model.add(x_expr[2*i]==1);
		model.add(x_expr[2*i+1]==1);
	}
	IloExprArray y_expr(env,N*_g.n);
	for(int m=0; m<_g.n*N; m++)
		y_expr[m]=IloExpr(env);
	for(int i=0; i<_g.n; i++)
		for(int j=0; j<N; j++)
		{
			for(int k=0; k<N; k++)
			{
				y_expr[i*N+j]+=x[k+N*i]*(int)g.A[k*N+j];
				if(k<_g.n)
				y_expr[i*N+j]-=(int)_g.A[i*_g.n+k]*x[k*N+j];
			}
			model.add(y_expr[i*N+j]>=0);
		}
}