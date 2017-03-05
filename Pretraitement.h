#ifndef DEF_H_PRETRAITEMENT
#define DEF_H_PRETRAITEMENT
#include "Graphe.h"
//#include <ilcplex/ilocplex.h>

bool pretraitement(Graphe const & g, Graphe const & _g, vector<vector<bool>> & x_mask, bool heur); // retourner true si le prétraitement révèle problème insoluble
void save_pretraitement(string path, vector<vector<bool>>const &);
bool load_pretraitement(string path, vector<vector<bool>> &);
void setArc(Graphe const &, Graphe const &, vector<vector<bool>>&, int, int, vector<int> &, int&);
void unsetArc(vector<vector<bool>>&, vector<int> &, int&);
void AC(Graphe const &, Graphe const &, vector<vector<bool>>&, vector<vector<int>>&);
void SC(Graphe const &, Graphe const &, vector<vector<bool>>&);


#endif