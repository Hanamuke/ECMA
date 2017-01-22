#include <iostream>
#include "Graphe.h"
#include "Pretraitement.h"
#include <ilcplex/ilocplex.h>
#define BENCH_FOLDER "../../benchmarks/" 
using namespace std;

int main()
{
	//string problem="test/";
	//string problem="si2_bvg_b03_200/si2_b03_m200.00/";
	string problem="si4_rand_r005_200/si4_r005_m200.04/";
    IloEnv env;
	/*Le prétraitement renvoie une matrice x_mask qui indique si x_ij peut être à un. On introduira seulement donc les termes nécessaires
	Il est possible q'on se rende déjà compte que le problème est insoluble à cette étape.*/

	try 
	{
		IloModel model(env);
		IloNumArray vals(env);
		IloBoolVarArray x(env);
		{ //ces accolades font restreigne la range des graphes et de x_mask, ils sont détruits à la fin des accolades, ils occuperont pas de la mémoire pendant la résolution.
			Graphe g;
			init(g,BENCH_FOLDER+problem+"target");

			Graphe g_barre;
			init(g_barre,BENCH_FOLDER+problem+"pattern");
			for(int i=0; i<g.n*g.n; i++)
				x.add(IloBoolVar(env));
			vector<vector<bool>> x_mask;
			pretraitement(g,g_barre,x_mask);
			loadModel(env,model,g,g_barre,x_mask,x);
		}
		IloCplex cplex(model);
		//cplex.setParam(IloCplex::NumParam::TiLim ,100);
		cplex.exportModel("projet.lp");
		cplex.solve();

		env.out() << "Solution status = " << cplex.getStatus() << endl;
		env.out() << "Solution value  = " << cplex.getObjValue() << endl;

		cplex.getValues(vals, x);
		//env.out() << "Values        = " << vals << endl;
		//cplex.exportModel("projet.lp");
	} catch (IloException& e) {
	cerr << "Concert Exception: " << e << endl;
	} catch (...) {
		cerr << "Other Exception" << endl;}
	env.end();
	cin.get();
    return 0;
}

