#include <iostream>
#include "Graphe.h"
#include "Pretraitement.h"
#include <ilcplex/ilocplex.h>
#define BENCH_FOLDER "../../benchmarks/" 
using namespace std;

int main()
{

	//string problem="scalefree/B.01/";
	string problem="si4_rand_r005_200/si4_r005_m200.04/";
    IloEnv env;
	Graphe g;
	g.A=NULL; //indispensable
	init(g,BENCH_FOLDER+problem+"target");
	Graphe g_barre;
	g_barre.A=NULL;
	init(g_barre,BENCH_FOLDER+problem+"pattern");

	vector<vector<bool>> x_mask;
	pretraitement(g,g_barre,x_mask);
	/*Le prétraitement renvoie une matrice x_mask qui indique si x_ij peut être à un. On introduira seulement donc les termes nécessaires
	Il est possible q'on se rende déjà compte que le problème est insoluble à cette étape.*/

	try 
	{
		IloModel model(env);
		IloNumArray vals(env);
		IloBoolVarArray x(env,g.n*g.n);
		loadModel(env,model,g,g_barre,x_mask,x);
		IloCplex cplex(model);
		cplex.setParam(IloCplex::NumParam::TiLim ,100);
		cplex.exportModel("projet.lp");
		cplex.solve();

		env.out() << "Solution status = " << cplex.getStatus() << endl;
		//env.out() << "Solution value  = " << cplex.getObjValue() << endl;

		cplex.getValues(vals, x);
		//env.out() << "Values        = " << vals << endl;
		//cplex.exportModel("projet.lp");
	} catch (IloException& e) {
	cerr << "Concert Exception: " << e << endl;
	} catch (...) {
		cerr << "Other Exception" << endl;}
	env.end();
	end(g); //indispensable
	end(g_barre);
	cin.get();
    return 0;
}

