#ifndef DEF_H_PRETRAITEMENT
#define DEF_H_PRETRAITEMENT
#include "Graphe.h"
//#include <ilcplex/ilocplex.h>

bool pretraitement(Graphe const & g, Graphe const & _g, vector<vector<bool>> & x_mask); // retourner true si le pr�traitement r�v�le probl�me insoluble
void save_pretraitement(string path, vector<vector<bool>>const &);
bool load_pretraitement(string path, vector<vector<bool>> &);

#endif