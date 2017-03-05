#include <iostream>
#include <fstream>
#include "Graphe.h"
#include "Pretraitement.h"
#include "Hopcroft.h"
#include <ilcp/cp.h>
#define BENCH_FOLDER "../benchmarks/" 
#define FORCE_PREPROCESSING
using namespace std;

bool loadModel(IloEnv, IloModel, Graphe const &, Graphe const &, vector<vector<bool>> const &, IloIntVarArray);
void toTulip(Graphe const&, string path);
void heuristique(Graphe const & g, Graphe const & _g, vector<vector<bool>> & x_mask, IloEnv env);



int main()
{
	
	
	bool heur=false;
	//string problem = "test/";
	// Instances à traiter
	//string problem = "si2_bvg_b03_200/si2_b03_m200.00/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.01/";
	//string problem = "si2_bvg_b03m_800/si2_b03m_m800.02/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.03/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.04/";
	//string problem = "si2_bvg_b03m_800/si2_b03m_m800.05/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.06/";
	string problem = "si2_bvg_b03m_400/si2_b03m_m400.07/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.08/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.09/";
		
	//string problem = "si4_rand_r005_200/si4_r005_m200.04/";
	//string problem="scalefree/F.01/";
	IloEnv env;
	/*Le prétraitement renvoie une matrice x_mask qui indique si x_ij peut être à un. On introduira seulement donc les termes nécessaires
	Il est possible q'on se rende déjà compte que le problème est insoluble à cette étape.*/

	try
	{
		
		//{ //ces accolades font restreingnent la range des graphes et de x_mask, ils sont détruits à la fin des accolades, ils occuperont pas de la mémoire pendant la résolution.
		Graphe g;
		init(g, BENCH_FOLDER + problem + "target");
		Graphe g_barre;
		init(g_barre, BENCH_FOLDER + problem + "pattern");
		//toTulip(g, BENCH_FOLDER + problem + "tp_target.tlp");
		//toTulip(g_barre, BENCH_FOLDER + problem + "tp_patter.tlp");
		
		vector<vector<bool>> x_mask;
#ifdef FORCE_PREPROCESSING
		pretraitement(g, g_barre, x_mask, heur);
		save_pretraitement(BENCH_FOLDER + problem + "mask", x_mask);
#endif
#ifndef FORCE_PREPROCESSING
		if (!load_pretraitement(BENCH_FOLDER + problem + "mask", x_mask))
		{
			pretraitement(g, g_barre, x_mask);
			save_pretraitement(BENCH_FOLDER + problem + "mask", x_mask);
		}
#endif	

		if(!heur){

			IloModel model(env);
			IloIntArray vals(env);
			IloIntVarArray x(env);
			for (int i = 0; i < g_barre.n; i++)
				x.add(IloIntVar(env));
			model.add(x);

			bool solvable=loadModel(env, model, g, g_barre, x_mask, x);
			IloCP cp(model);

			if(!solvable)
				cout<<"Aucune Solution"<<endl;
			else if(cp.solve()){

				env.out() << "Solution retour = " << cp.getStatus() << endl;			
				for(int i=0; i<g_barre.n; i++){
					cp.out()<< i<<" : "<<cp.getValue(x[i])<<endl;
				}

			} 
		}
		else
			heuristique(g, g_barre,  x_mask, env);
		
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

bool loadModel(IloEnv env, IloModel model, Graphe const & g, Graphe const & _g, vector<vector<bool>> const & x_mask, IloIntVarArray x)
{
	
	for(int i=0; i<_g.n; i++){
		IloIntArray domain(env);
		for(int j=0; j<g.n; j++)
			if(x_mask[i][j])
				domain.add(j);
		if(domain.getSize()>0){
			x[i].setPossibleValues(domain);
		}
		else
			return false;
	}

	model.add(IloAllDiff(env, x));

	for(int i=0; i<_g.n; i++)
		for(int j=0; j<_g.n; j++)
			if(_g.A[i].test(j)){
				IloIntArray domaini(env);
				x[i].getPossibleValues(domaini);
				IloIntArray domainj(env);
				x[j].getPossibleValues(domainj);
				for(int iter=0; iter<domaini.getSize(); iter++){
					IloInt y=domaini[iter];
					IloOr inter(env);
					int vide=true;
					for(int k=0; k<g.n; k++)
						if(g.A[y].test(k) && x_mask[j][k]){
							vide=false;
							inter.add(x[j]==k);
						}
					
					if(vide)
						model.add(x[i]!=y);
					else
						model.add(IloIfThen(env, x[i]==y, inter));
				}
			}

	return true;
}

void toTulip(Graphe const& g, string path)
{
	fstream f(path, fstream::out | fstream::trunc);
	f << "(tlp \"2.0\"" << endl;
	f << "(nodes";
	for (int i = 0; i < g.n; i++)
		f << " " << i;
	f << ")" << endl;
	int k = 0;
	for (int i = 0; i < g.n; i++)
		for (int j = i + 1; j < g.n; j++)
			if (g.A[i][j])
			{
				f << "(edge " << k << " " << i << " " << j << ")" << endl;
				k++;
			}
	f << ")";
}

void verification(IloCP cp, IloIntVarArray & x, Graphe const & g, Graphe const & _g){
	int verif=true;
	for(int i=0; i<_g.n; i++)
		for(int j=0; j<_g.n; j++)
			if(_g.A[i].test(j)){
				int imi=cp.getValue(x[i]);
				int imj=cp.getValue(x[j]);
				verif=verif&&(g.A[imi].test(imj));
			}
	cout<<"Verif : "<<verif<<endl;

}


void heuristique(Graphe const & g, Graphe const & _g, vector<vector<bool>> & x_mask, IloEnv env){
		
		
		int N=g.n;
		int _N=_g.n;

		
		int minn=N+1;
		int argi=-1;
		minn=N+1;
		for(int i=0; i<_N; i++){
			
			int compt=0;
			for(int j=0; j<N; j++)
				if(x_mask[i][j])
					compt++;
			if(compt<minn && compt>1){
				minn=compt;
				argi=i;
			}
		}
		cout<<"argi : "<<argi<<", minn : "<<minn<<endl;

		for(int k=0; k<10; k++){
			if(x_mask[argi][k]){
				//Preparation du (Un)SetArc
				vector<int> f_mask;
				f_mask.resize(g.n*_g.n);
				int ptr = 0;
				vector<vector<int>> nbCouplage;
				nbCouplage.resize(_N);
				for (int i = 0; i < _N; i++)
				{
					x_mask[i].resize(N);
					nbCouplage[i].resize(N);
				}


				setArc(g, _g, x_mask, argi, k, f_mask,ptr);
				minn=N+1;
				while(minn>N/100){
					int argi2=-1;
					minn=N+1;
					for(int i=0; i<_N; i++){
				
						int compt=0;
						for(int j=0; j<N; j++)
							if(x_mask[i][j])
								compt++;
						if(compt<minn && compt>1){
							minn=compt;
							argi2=i;
						}
					}

					int argj=rand()%N;
					while(!x_mask[argi2][argj])
						if(argj==N-1)
							argj=0;
						else
							argj++;
			
			
					cout<<"argi2 : "<<argi2<<", argj : "<<argj<<" minn : "<<minn<<endl;
					setArc(g, _g, x_mask, argi2, argj, f_mask,ptr);
				}
				cout<<"Debut AC"<<endl;
				vector<vector<bool>> essai=x_mask;
				AC(g,_g,essai, nbCouplage); //propagation sur une copie
				


				IloModel model(env);
				IloIntArray vals(env);
				IloIntVarArray x(env);
				for (int i = 0; i < _N; i++)
					x.add(IloIntVar(env));
				model.add(x);
				bool solvable=loadModel(env, model, g, _g, essai, x);
				IloCP cp(model);

				if(solvable && cp.solve()){

					env.out() << "Solution retour = " << cp.getStatus() << endl;			
					for(int i=0; i<_N; i++){
						cp.out()<< i<<" : "<<cp.getValue(x[i])<<endl;
					}
					verification(cp, x, g, _g);
				} 
				else{
					cout<<"Pas de sol avec ce choix"<<endl;
					unsetArc(x_mask, f_mask, ptr);
				}
			}
		}

		
		

		
}
