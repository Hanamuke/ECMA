#include <iostream>
#include <fstream>
#include "Graphe.h"
#include "Pretraitement.h"
#include <ilcplex/ilocplex.h>
#define BENCH_FOLDER "benchmarks/" 
//#define FORCE_PREPROCESSING
using namespace std;

void loadModel(IloEnv, IloModel , Graphe const & , Graphe const & ,vector<vector<bool>> const & ,IloBoolVarArray, IloExprArray &, vector<vector<int>> & );

ILOSTLBEGIN

ILOUSERCUTCALLBACK5(CutCallback, IloBoolVarArray, x, IloExprArray, y_expr, vector<vector<int>>, y_indices, int, N, int, _N){

   IloEnv masterEnv = getEnv();
   IloInt numEdges = x.getSize();

   // Get the current x solution
  
   IloBoolArray xSol(masterEnv, numEdges);
   for (int i = 0; i < numEdges; ++i) {
	   xSol[i]=getValue(x[i]);
   }

   int nbAjoute=0;

   for(int i=0; i<_N; i++)
		{
			for(int j=0; j<N; j++)
			{
				IloInt total=0;
				IloInt taille=y_indices[i*N+j].size();
				for(int k=0; k<taille; k++){
					int ind=y_indices[i*N+j][k];
					if(ind<0)
						total-=xSol[-ind-1];
					else
						total+=xSol[ind-1];
				}
				if(total<0){
					nbAjoute++;
					try{
						add(y_expr[i*N+j]>=0).end();
					}catch(IloException e){
						throw e;
					}
				}
			}
		}

   cout<<"Nombre Ajoute :"<<nbAjoute<<endl;
}

ILOLAZYCONSTRAINTCALLBACK5(LazyCallback, IloBoolVarArray, x, IloExprArray, y_expr, vector<vector<int>>, y_indices, int, N, int, _N){

   IloEnv masterEnv = getEnv();
   IloInt numEdges = x.getSize();

   // Get the current x solution
  
   IloBoolArray xSol(masterEnv, numEdges);
   for (int i = 0; i < numEdges; ++i) {
	   xSol[i]=getValue(x[i]);
   }

   int nbAjoute=0;

   for(int i=0; i<_N; i++)
		{
			for(int j=0; j<N; j++)
			{
				IloInt total=0;
				IloInt taille=y_indices[i*N+j].size();
				for(int k=0; k<taille; k++){
					int ind=y_indices[i*N+j][k];
					if(ind<0)
						total-=xSol[-ind-1];
					else
						total+=xSol[ind-1];
				}
				if(total<0){
					nbAjoute++;
					try{
						add(y_expr[i*N+j]>=0).end();
					}catch(IloException e){
						throw e;
					}
				}
			}
		}

   cout<<"Nombre Ajoute :"<<nbAjoute<<endl;
   
   


}



int main()
{

	//string problem="test/";
	string problem = "si2_bvg_b03_400/si2_b03_m400.02/";
	//string problem="si4_rand_r005_200/si4_r005_m200.04/";
	//string problem="scalefree/F.01/";
	IloEnv env;
	/*Le prétraitement renvoie une matrice x_mask qui indique si x_ij peut être à un. On introduira seulement donc les termes nécessaires
	Il est possible q'on se rende déjà compte que le problème est insoluble à cette étape.*/

	try
	{
		IloModel model(env);
		IloNumArray vals(env);
		IloBoolVarArray x(env);

		int N;
		int _N;
		IloExprArray y_expr(env);
		vector<vector<int>> y_indices;

		{ //ces accolades font restreigne la range des graphes et de x_mask, ils sont détruits à la fin des accolades, ils occuperont pas de la mémoire pendant la résolution.
			Graphe g;
			init(g, BENCH_FOLDER + problem + "target");
			N=g.n;
			Graphe g_barre;
			init(g_barre, BENCH_FOLDER + problem + "pattern");
			_N=g_barre.n;
			for (int i = 0; i < g.n*g_barre.n; i++)
				x.add(IloBoolVar(env));
			vector<vector<bool>> x_mask;
#ifdef FORCE_PREPROCESSING
			pretraitement(g, g_barre, x_mask);
			save_pretraitement(BENCH_FOLDER + problem + "mask", x_mask);
#endif
#ifndef FORCE_PREPROCESSING
			if (!load_pretraitement(BENCH_FOLDER + problem + "mask", x_mask))
			{
				pretraitement(g, g_barre, x_mask);
				save_pretraitement(BENCH_FOLDER + problem + "mask", x_mask);
			}
#endif
			loadModel(env,model,g,g_barre,x_mask,x, y_expr, y_indices);
		}

		
		IloCplex cplex(model);
		//cplex.setParam(IloCplex::NumParam::TiLim ,100);

		cplex.use(CutCallback(env,x,y_expr,y_indices,N,_N));
		cplex.use(LazyCallback(env,x,y_expr,y_indices,N,_N));

		cplex.exportModel("projet.lp");
		cplex.solve();

		env.out() << "Solution status = " << cplex.getStatus() << endl;
		env.out() << "Solution value  = " << cplex.getObjValue() << endl;

		cplex.getValues(vals, x);
		//env.out() << "Values        = " << vals << endl;
		//cplex.exportModel("projet.lp");
	}
	catch (IloException& e) {
		cerr << "Concert Exception: " << e << endl;
	}
	catch (...) {
		cerr << "Other Exception" << endl;
	}
	env.end();
	cin.get();
	return 0;
}

void loadModel(IloEnv env, IloModel model, Graphe const & g, Graphe const & _g, vector<vector<bool>> const & x_mask, IloBoolVarArray x,
	IloExprArray & y_expr, vector<vector<int>> & y_indices)
{
	int N = g.n, _N = _g.n;
	IloExprArray ligne(env, _N);
	for (int m = 0; m < _N; m++)
		ligne[m] = IloExpr(env);
	IloExprArray collonne(env, N);
	for (int m = 0; m < N; m++)
		collonne[m] = IloExpr(env);


	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			if (i < _N && x_mask[i][j] )
				ligne[i] += x[i*N + j];
			if (j < _N && x_mask[j][i])
				collonne[i] += x[j*N + i];
			if (i < _N && !x_mask[i][j])
				model.add(x[i*N + j] == 0); //contrainte liées au prétraitement
		}
		//contraintes de X matrice de transposition, contrainte paire sur les colonnes et contraintes impaires sur les lignes (je crois)
		if (i < _N)
			model.add(ligne[i] == 1);
		model.add(collonne[i] <= 1);
	}

	y_expr=IloExprArray(env,N*_g.n);
	
	for(int m=0; m<_g.n*N; m++){
		y_expr[m]=IloExpr(env);
		vector<int> v;
		y_indices.push_back(v);
	}
	for(int i=0; i<_N; i++)
		for(int j=0; j<N; j++)
		{
			for(int k=0; k<N; k++)
			{
				if(x_mask[i][k] && g.A[k].test(j)){
					y_expr[i*N+j]+=x[i*N+k];
					y_indices[i*N+j].push_back(i*N+k+1);
				}
				if(k<_N && x_mask[k][j] && _g.A[i].test(k)){
					y_expr[i*N+j]-=x[k*N+j];
					y_indices[i*N+j].push_back(-(k*N+j+1));
				}
			}
			//model.add(y_expr[i*N+j]>=0);
		}
}
