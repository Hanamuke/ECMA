#ifndef DEF_H_PRETRAITEMENT
#define DEF_H_PRETRAITEMENT
#include "Graphe.h"
#include <ilcplex/ilocplex.h>

bool pretraitement(Graphe const & g, Graphe const & _g,vector<vector<bool>> & x_mask); // retourner true si le prétraitement révèle problème insoluble
void print(vector<vector<bool>> & x_mask);

#endif