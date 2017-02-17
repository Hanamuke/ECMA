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

void initialiseAleatoirement(vector<vector<bool>> & x_mask, vector<vector<int>> & colonnes, int nbSolutions);
//la valeur du retour est la valeur associee au dual de la contrainte de convexite
void valeursDuales(vector<vector<int>> & colonnes, vector<vector<bool>> & x_mask, Graphe & g, Graphe & _g, IloEnv & env, 
	IloNumVarArray & lambda, IloExpr & obj, IloExprArray & expr, IloExpr & convexite);
void ajouteColonne(IloNumArray & duals,vector<vector<bool>> & x_mask, vector<vector<int>> & colonnes);
bool estRealisable(vector<int> & solution, Graphe & g, Graphe & _g);
void initialiseModel(vector<vector<int>> & colonnes, Graphe & g, Graphe & _g, IloEnv & env, 
	IloNumVarArray & lambda, IloExpr & obj, IloExprArray & expr, IloExpr & convexite);



int main()
{
	string problem="test/";
	//string problem = "si2_bvg_b03_400/si2_b03_m400.02/";
	//string problem="si4_rand_r005_200/si4_r005_m200.04/";
	//string problem="scalefree/F.01/";
	IloEnv env;
	/*Le pr�traitement renvoie une matrice x_mask qui indique si x_ij peut �tre � un. On introduira seulement donc les termes n�cessaires
	Il est possible q'on se rende d�j� compte que le probl�me est insoluble � cette �tape.*/

	//srand(time(NULL));

	try
	{

		int N,_N;
		vector<vector<bool>> x_mask;
		Graphe g;
		init(g, BENCH_FOLDER + problem + "target");
		N=g.n;
		Graphe g_barre;
		init(g_barre, BENCH_FOLDER + problem + "pattern");
		_N=g_barre.n;
		{ //ces accolades font restreigne la range des graphes et de x_mask, ils sont d�truits � la fin des accolades, ils occuperont pas de la m�moire pendant la r�solution.
		

			
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

		vector<int> couplageInit;
		for(int i=0; i<_N; i++)
			couplageInit.push_back(-1);
		colonnes.push_back(couplageInit);
	
		//On construit _N solutions intiales qui sont toutes nulles � l'exception d'un xij par solution qui est non nul :
		// inutile car elles ne sont pas r�alisables
	/*	for(int i=0; i<_N; i++){
			int alea=rand()%N;
			while(!x_mask[i][alea])
				alea=rand()%N;

			couplageInit[i]=alea;
			colonnes.push_back(couplageInit);
			couplageInit[i]=-1;
		}
		*/



		/*int nbSolutions=5;
		initialiseAleatoirement(x_mask, colonnes, nbSolutions);*/
		
		for(unsigned int ne=0; ne<colonnes.size(); ne++){
			for(int i=0; i<_N; i++)
					cout<<colonnes[ne][i]<<" ";
			cout<<endl;
		}

		IloEnv env;
		IloNumVarArray lambda(env);
		IloExpr obj(env);
		IloExprArray expr(env, _N*N);
		for(int i=0; i<_N*N; i++)
			expr[i]=IloExpr(env);
		IloExpr convexite=IloExpr(env);
		
		//initialiseModel(colonnes, g, g_barre, env, lambda, obj, expr, convexite);

		int compteur=0;
		bool trouve=false;
		while(compteur<300){ //!trouve){
			valeursDuales(colonnes, x_mask, g, g_barre, env, lambda, obj, expr, convexite);
			trouve=estRealisable(colonnes.back(), g, g_barre);
			cout<<"ALors ?"<<trouve<<endl;
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

bool estRealisable(vector<int> & solution, Graphe & g, Graphe & _g){
	
	bool retour=true;
	int _N=_g.n;
	int N=g.n;

	vector<int> xij;
	xij.resize(N);
	int nbrMarie=0;
	for(int i=0; i<N; i++)
		xij[i]=-1;
	for(int i=0; i<_N; i++)
		if(solution[i]!=-1){
			xij[solution[i]]=i;
			nbrMarie++;
		}

	retour=retour && (nbrMarie==_N);
	for(int i=0; i<_N; i++)
		for(int j=0; j<N; j++){
			int lprime = solution[i];
			int l = xij[j];

			if(l!=-1){
				int val = _g.A[i][l]-g.A[lprime][j];
				retour = retour && ((_g.A[i][l]-g.A[lprime][j])<=0);
			}
			else{
				int val = -g.A[lprime][j];
				retour = retour && (-g.A[lprime][j]<=0);
			}

			//cout<<"ici"<<retour<<endl;
		}

	return retour;
}

void initialiseAleatoirement(vector<vector<bool>> & x_mask, vector<vector<int>> & colonnes, int nbSolution){
	
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
	//cout<<"Coeffs : "<<endl;
	for(int i=0; i<_N; i++){
		in[i].resize(N);
		for(int j=0; j<N; j++){
			if(x_mask[i][j]==0)
				in[i][j]=-INT_MAX/2;
			else
				in[i][j]=duals[i*N+j];
			//cout<<" "<<in[i][j];
		}
		cout<<endl;
	}

	
	
	bool fin=false;
	while(!fin){
		vector<int> ret;
		khun(in, ret);

		//On v�rifie que la colonne entrante n'est pas d�j� pr�sente
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
			if(identique)
				different=false;
			else
				it++;
		}
		if(different || m==1){
			// Si la colonne n'est pas d�j� pr�sente
			fin=true;
			colonnes.push_back(ret);
		}
		else{
			// Sinon cela signifie que le a colonne entrante est d�j� pr�sente
			// Dans ce cas on supprime l'ar�te du couplage qui est la moins rentable et on cherche un autre couplage
			cout<<"Deja Present"<<endl;

			int min=INT_MAX/2;
			int argmin=-1;
			int comptMin=0;
			for(int i=0; i<_N; i++)
				if(in[i][ret[i]]<min){
					min=in[i][ret[i]];
					argmin=i;
					comptMin=1;
				}
				else if(in[i][ret[i]]==min){
					comptMin++;
					int alea=rand()%comptMin;
					if(alea==0){
						min=in[i][ret[i]];
						argmin=i;
					}
				}
			in[argmin][ret[argmin]]=-INT_MAX/2;

			/*cout<<"NOuveaux coeffs :"<<endl;
			for(int i=0; i<_N; i++){
				for(int j=0; j<N; j++)
					cout<<" "<<in[i][j];
				cout<<endl;
			}*/


		}
		
	}
}

void initialiseModel(vector<vector<int>> & colonnes, Graphe & g, Graphe & _g, IloEnv & env, 
	IloNumVarArray & lambda, IloExpr & obj, IloExprArray & expr, IloExpr & convexite){
	
	int m = colonnes.size();
	int _N=_g.n;
	int N=g.n;
	
	

	for(int k=0; k<m-1; k++){
		lambda.add(IloNumVar(env));
		// Traitement pour savoir quel sommet de G recoit quel sommet de _G
		vector<int> xij;
		xij.resize(N);
		int nbrMarie=0;
		for(int i=0; i<N; i++)
			xij[i]=-1;
		for(int i=0; i<_N; i++)
			if(colonnes[k][i]!=-1){
				xij[colonnes[k][i]]=i;
				nbrMarie++;
			}


		// L'objectif (nbrMarie vaut _N si le couplage est complet et 0 sinon (pour le couplage nul)) :
		obj+=lambda[k]*nbrMarie;
		convexite+=lambda[k];

		//Les contraintes :
		for(int i=0; i<_N; i++)
			for(int j=0; j<N; j++){
				int lprime = colonnes[k][i];
				int l = xij[j];

				if(l!=-1 && lprime!=-1)
					expr[i*N+j]+=(_g.A[i][l]-g.A[lprime][j])*lambda[k];
				else if (lprime!=-1)
					expr[i*N+j]-=g.A[lprime][j]*lambda[k];
				else if (l!=-1)
					expr[i*N+j]+=_g.A[i][l]*lambda[k];

			}


	}


}


void valeursDuales(vector<vector<int>> & colonnes, vector<vector<bool>> & x_mask, Graphe & g, Graphe & _g, IloEnv & env, IloNumVarArray & lambda, IloExpr & obj, IloExprArray & expr, IloExpr & convexite){

	int m = colonnes.size();
	cout<<"m : "<<m<<endl;
	int _N=x_mask.size();
	int N=x_mask[0].size();
	
	lambda.add(IloNumVar(env));
	int k = m-1;

	// Traitement pour savoir quel sommet de G recoit quel sommet de _G
	vector<int> xij;
	xij.resize(N);
	int nbrMarie=0;
	for(int i=0; i<N; i++)
		xij[i]=-1;
	for(int i=0; i<_N; i++)
		if(colonnes[k][i]!=-1){
			xij[colonnes[k][i]]=i;
			nbrMarie++;
		}
	

	// L'objectif (nbrMarie vaut _N si le couplage est complet et 0 sinon (pour le couplage nul)) :
	obj+=lambda[k]*nbrMarie;
	//Les contraintes :

	for(int i=0; i<_N; i++)
		for(int j=0; j<N; j++){
			int lprime = colonnes[k][i];
			int l = xij[j];

			if(l!=-1 && lprime!=-1)
				expr[i*N+j]+=(_g.A[i][l]-g.A[lprime][j])*lambda[k];
			else if (lprime!=-1)
				expr[i*N+j]-=g.A[lprime][j]*lambda[k];
			else if (l!=-1)
				expr[i*N+j]+=_g.A[i][l]*lambda[k];
		}
		//cout<<"i,j :"<<i<<","<<j<<", expr : "<<expr[i*N+j]<<", l : "<< l <<", lprime : "<<lprime<<endl;

			
		
	

	//cout<<"Contraintes : "<<endl;
	IloRangeArray constraints(env);
	for(int i=0; i<_N; i++)
		for(int j=0; j<N; j++){
			constraints.add(expr[i*N+j]<=0);
			//cout<<"i,j :"<<i<<","<<j<<", constaintes : "<<constraints[i*N+j]<<endl;
		}

	
	convexite+=lambda[k];
	constraints.add(convexite==1);

	
	IloModel model(env);
	model.add(constraints);
	model.add(IloMaximize(env, obj));

	//IloNumArray vals(env);
	//R�solution : 

	IloCplex cplex(model);
	cplex.solve();
	env.out() << "Solution status = " << cplex.getStatus() << endl;
	env.out() << "Solution value  = " << cplex.getObjValue() << endl;
//	cplex.getValues(vals, lambda);
	//env.out() << "Values        = " << vals << endl;
	IloNumArray duals(env);
	cplex.getDuals(duals, constraints);
	env.out()<<" Duals = "<<duals<<endl;

	ajouteColonne(duals, x_mask, colonnes);


}