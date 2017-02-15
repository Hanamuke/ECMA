#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include "Graphe.h"
#include "Pretraitement.h"
#include "Khun.h"
#include <ilcplex/ilocplex.h>
#define BENCH_FOLDER "benchmarks/" 
//#define FORCE_PREPROCESSING
using namespace std;

void initialise(vector<vector<bool>> & x_mask, vector<vector<int>> & colonnes, int nbSolutions);
//la valeur du retour est la valeur associee au dual de la contrainte de convexite
int valeursDuales(vector<vector<int>> & colonnes, vector<vector<bool>> & x_mask, Graphe & g, Graphe & _g);
void ajouteColonne(IloNumArray & duals,vector<vector<bool>> & x_mask, vector<vector<int>> & colonnes);



int mainbis()
{
	string problem="test/";
	//string problem = "si2_bvg_b03_400/si2_b03_m400.02/";
	//string problem="si4_rand_r005_200/si4_r005_m200.04/";
	//string problem="scalefree/F.01/";
	IloEnv env;
	/*Le prétraitement renvoie une matrice x_mask qui indique si x_ij peut être à un. On introduira seulement donc les termes nécessaires
	Il est possible q'on se rende déjà compte que le problème est insoluble à cette étape.*/

	//srand(time(NULL));

	try
	{
		IloModel model(env);
		IloNumArray vals(env);
		IloBoolVarArray x(env);
		int N,_N;
		vector<vector<bool>> x_mask;
		Graphe g;
		init(g, BENCH_FOLDER + problem + "target");
		N=g.n;
		Graphe g_barre;
		init(g_barre, BENCH_FOLDER + problem + "pattern");
		_N=g_barre.n;
		{ //ces accolades font restreigne la range des graphes et de x_mask, ils sont détruits à la fin des accolades, ils occuperont pas de la mémoire pendant la résolution.
		
			for (int i = 0; i < g.n*g_barre.n; i++)
				x.add(IloBoolVar(env));
			
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
		}

		cout<<"x_mask"<<endl;
		for(int i=0; i<_N; i++){
			for(int j=0; j<N; j++)
				cout<<x_mask[i][j]<<" ";
			cout<<endl;
		}
		
		vector<vector<int>> colonnes;

		vector<int> couplageNul;
		for(int i=0; i<_N; i++)
			couplageNul.push_back(-1);
		colonnes.push_back(couplageNul);

		//int nbSolutions=5;
		//initialise(x_mask, colonnes, nbSolutions);
		
		for(unsigned int ne=0; ne<colonnes.size(); ne++){
			for(int i=0; i<_N; i++)
					cout<<colonnes[ne][i]<<" ";
			cout<<endl;
		}



		int compteur=0;
		while(compteur<20){
			valeursDuales(colonnes, x_mask, g, g_barre);
			/*vector<int> v=colonnes.back();
			for(int i=0; i<_N; i++)
					cout<<v[i]<<" ";
			cout<<endl;*/
			compteur++;
		}
	
	}catch (IloException& e) {
		cerr << "Concert Exception: " << e << endl;
	}
	catch (...) {
		cerr << "Other Exception" << endl;
	}
	env.end();
	cin.get();
	return 0;
}



void initialise(vector<vector<bool>> & x_mask, vector<vector<int>> & colonnes, int nbSolution){
	
	int _N=x_mask.size();
	int N=x_mask[0].size();
	
	for(int ne=0; ne<nbSolution; ne++){
		vector<vector<int>> in;
		in.resize(_N);
		for(int i=0; i<_N; i++){
			in[i].resize(N);
			for(int j=0; j<N; j++)
				in[i][j]=x_mask[i][j]*rand()%3;
		}

		vector<int> ret;
		khun(in, ret);
		colonnes.push_back(ret);
	}

}


void ajouteColonne(IloNumArray & duals,vector<vector<bool>> & x_mask, vector<vector<int>> & colonnes){
	int _N=x_mask.size();
	int N=x_mask[0].size();

	vector<vector<int>> in;
	in.resize(_N);
	cout<<"Coeffs : "<<endl;
	for(int i=0; i<_N; i++){
		in[i].resize(N);
		for(int j=0; j<N; j++){
			in[i][j]=x_mask[i][j]*(1+duals[i*N+j]);
			cout<<" "<<in[i][j];
		}
		cout<<endl;
	}

	vector<int> ret;
	khun(in, ret);
	
	bool fin=false;
	do{
		//On vérifie que la colonne entrante n'est pas déjà présente
		bool different=true;

		cout<<"ret :";
				for(int i=0; i<_N; i++)
				cout<<" " <<ret[i];
		cout<<endl;

		int it=1;
		int m=colonnes.size();
		while(it<m && different){
			bool identique=true;
			int iter=0;
			while(iter<_N && identique){
				identique=identique&&(colonnes[it][iter]==ret[iter]);
				iter++;
			}
			if(iter==_N)
				different=false;
			else
				it++;
		}
		if(different || m==1){
			fin=true;
			colonnes.push_back(ret);
		}
		else{
			// Sinon cela signifie que le a colonne entrante est déjà présente
			// Dans ce cas on supprime l'arête du couplage qui est la moins rentable et on cherche un autre couplage
			cout<<"Deja Present"<<endl;

			int min=80000000;
			int argmin=-1;
			for(int i=0; i<_N; i++)
				if(in[i][ret[i]]<min){
					min=in[i][ret[i]];
					argmin=i;
					cout<<"in[i][ret[i]] :" <<in[i][ret[i]]<<", i :"<<i<<endl;
				}
			in[argmin][ret[argmin]]=0;

			for(int i=0; i<_N; i++){
				for(int j=0; j<N; j++)
					cout<<" "<<in[i][j];
				cout<<endl;
			}

			khun(in, ret);
		}
		
	}while(!fin);
}



int valeursDuales(vector<vector<int>> & colonnes, vector<vector<bool>> & x_mask, Graphe & g, Graphe & _g){

	int m = colonnes.size();
	cout<<"m : "<<m<<endl;
	int _N=x_mask.size();
	int N=x_mask[0].size();
	IloEnv env;
	IloModel model(env);
	IloNumArray vals(env);
	IloNumVarArray lambda(env);
	
	for(int i=0; i<m; i++)
		lambda.add(IloNumVar(env));


	// Les contraintes et l'objectif :

	IloExpr obj(env);
	IloExprArray expr(env, _N*N);
	for(int i=0; i<_N*N; i++)
		expr[i]=IloExpr(env);

	for(int k=1; k<m; k++){


		// Traitement pour savoir quel sommet de G recoit quel sommet de _G
		vector<int> xij;
		xij.resize(N);
		int nbrMarie=0;
		for(int i=0; i<N; i++)
			xij[i]=-1;
		for(int i=0; i<_N; i++){
			if(colonnes[k][i]!=-1){
				xij[colonnes[k][i]]=i;
				nbrMarie++;
			}
		}

		// L'objectif (nbrMarie vaut _N si le couplage est complet et 0 sinon (pour le couplage nul)) :
		obj+=lambda[k]*nbrMarie;

		//Les contraintes :
		for(int i=0; i<_N; i++){
			for(int j=0; j<N; j++){
				int lprime = colonnes[k][i];
				int l = xij[j];
				//if(x_mask[i][j]){
					if(l!=-1)
						expr[i*N+j]-=(_g.A[i][l]-g.A[lprime][j])*lambda[k];
					else
						expr[i*N+j]+=+g.A[lprime][j]*lambda[k];
				//}
				//cout<<"i,j :"<<i<<","<<j<<", expr : "<<expr[i*N+j]<<", l : "<< l <<", lprime : "<<lprime<<endl;
			}
		}
	}


	//cout<<"Contraintes : "<<endl;
	IloRangeArray constraints(env);
	for(int i=0; i<_N; i++){
		for(int j=0; j<N; j++){
			constraints.add(expr[i*N+j]<=0);
			//cout<<"i,j :"<<i<<","<<j<<", constaintes : "<<constraints[i*N+j]<<endl;
		}
		cout<<endl;
	}
	

	
	IloExpr convexite=IloExpr(env);
	for(int i=0; i<m; i++)
		convexite+=lambda[i];

	constraints.add(convexite==1);
	model.add(constraints);

	model.add(IloMaximize(env, obj));

	//Résolution : 

	IloCplex cplex(model);
	cplex.solve();
	env.out() << "Solution status = " << cplex.getStatus() << endl;
	env.out() << "Solution value  = " << cplex.getObjValue() << endl;
	cplex.getValues(vals, lambda);
	env.out() << "Values        = " << vals << endl;
	IloNumArray duals(env);
	cplex.getDuals(duals, constraints);
	env.out()<<" Duals = "<<duals<<endl;

	ajouteColonne(duals, x_mask, colonnes);

	return 0;

}