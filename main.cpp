#include <iostream>
#include <fstream>
#include "Graphe.h"
#include "Pretraitement.h"
#include "Hopcroft.h"
#include <ilcp/cp.h>
#define BENCH_FOLDER "../benchmarks/" 
#define FORCE_PREPROCESSING
using namespace std;

void loadModel(IloEnv, IloModel, Graphe const &, Graphe const &, vector<vector<bool>> const &, IloIntVarArray);
void toTulip(Graphe const&, string path);
void intersection(IloIntArray & a, IloIntArray & b, IloOr & retour, IloIntVar & x);



int main()
{
	//string problem = "test/";
	// Instances à traiter
	//string problem = "si2_bvg_b03_800/si2_b03_m800.00/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.01/";
	string problem = "si2_bvg_b03_800/si2_b03_m800.02/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.03/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.04/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.05/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.06/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.07/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.08/";
	//string problem = "si2_bvg_b03_800/si2_b03_m800.09/";
		
	//string problem = "si4_rand_r005_200/si4_r005_m200.04/";
	//string problem="scalefree/F.01/";
	IloEnv env;
	/*Le prétraitement renvoie une matrice x_mask qui indique si x_ij peut être à un. On introduira seulement donc les termes nécessaires
	Il est possible q'on se rende déjà compte que le problème est insoluble à cette étape.*/

	try
	{
		IloModel model(env);
		IloIntArray vals(env);
		IloIntVarArray x(env);
		//{ //ces accolades font restreingnent la range des graphes et de x_mask, ils sont détruits à la fin des accolades, ils occuperont pas de la mémoire pendant la résolution.
		Graphe g;
		init(g, BENCH_FOLDER + problem + "target");
		Graphe g_barre;
		init(g_barre, BENCH_FOLDER + problem + "pattern");
		//toTulip(g, BENCH_FOLDER + problem + "tp_target.tlp");
		//toTulip(g_barre, BENCH_FOLDER + problem + "tp_patter.tlp");
		for (int i = 0; i < g_barre.n; i++)
			x.add(IloIntVar(env));
		model.add(x);
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
		
		/*for(int i=0; i<g.n; i++){
			for(int j=0; j<g.n; j++)
				cout<<g.A[i][j]<<" ";
			cout<<endl;
		}*/

		loadModel(env, model, g, g_barre, x_mask, x);
		IloCP cp(model);


		if(cp.solve()){

			env.out() << "Solution retour = " << cp.getStatus() << endl;			
			for(int i=0; i<g_barre.n; i++){
				cp.out()<< i<<" : "<<cp.getValue(x[i])<<endl;
			}

		} 
		
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

void loadModel(IloEnv env, IloModel model, Graphe const & g, Graphe const & _g, vector<vector<bool>> const & x_mask, IloIntVarArray x)
{
	
	for(int i=0; i<_g.n; i++){
		IloIntArray domain(env);
		for(int j=0; j<g.n; j++)
			if(x_mask[i][j])
				domain.add(j);
		x[i].setPossibleValues(domain);
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
					IloIntArray voisin(env);
					IloInt y=domaini[iter];
					for(int k=0; k<g.n; k++)
						if(g.A[y].test(k))
							voisin.add(k);
					IloOr inter(env);

					intersection(domainj, voisin, inter, x[j]);
					model.add(IloIfThen(env, x[i]==y, inter));
				}
			}

			
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


void intersection(IloIntArray & a, IloIntArray & b, IloOr & retour, IloIntVar & x){
	int sa=a.getSize();
	int sb=b.getSize();
	int posa=0;
	int posb=0;
	bool fin=false;

	while((posa<sa || posb<sb) && !fin){
		if(a[posa]==b[posb]){
			retour = retour || x==a[posa] ;
			posa++;
			if(posa==sa)
				fin=true;
			posb++;
			if(posb==sb)
				fin=true;
		}
		else if(posa==sa-1 && a[posa]<b[posb])
			fin=true;
		else if(posb==sb-1 && b[posb]<a[posa])
			fin=true;
		else if(a[posa]<b[posb])
			posa++;
		else if(b[posb]<a[posa])
			posb++;
	}

	/*
		cout<<endl;
	for(int i=0; i<sa; i++)
		cout<<a[i]<<" ";
	cout<<endl;
	for(int i=0; i<sb; i++)
		cout<<b[i]<<" ";
	cout<<endl;

	cout<<retour<<endl;*/
	
}

