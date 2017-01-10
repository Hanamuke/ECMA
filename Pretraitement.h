#ifndef DEF_H_PRETRAITEMENT
#define DEF_H_PRETRAITEMENT
#include "Graphe.h"
#include <ilcplex/ilocplex.h>

bool pretraitement(Graphe const & g, Graphe const & _g,vector<vector<bool>> & x_mask); // retourner true si le pr�traitement r�v�le probl�me insoluble
void loadModel(IloEnv env, IloModel model, Graphe const & g, Graphe const & _g,vector<vector<bool>> const & x_mask,IloBoolVarArray x);
void print(vector<vector<bool>> & x_mask);

#endif