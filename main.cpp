#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include "Graphe.h"
#include "Pretraitement.h"
#include "Khun.h"
#include <ilcplex/ilocplex.h>

using namespace std;

int main(){

	vector<vector<int>> in;
	vector<int> v;
	v.push_back(1);
	v.push_back(0);
	v.push_back(0);
	v.push_back(6);
	v.push_back(11);
	v.push_back(1);
	v.push_back(6);
	v.push_back(1);
	v.push_back(1);
	v.push_back(1);
	v.push_back(1);
	v.push_back(0);
	v.push_back(0);
	in.push_back(v);

	v[0]=0;
	v[1]=0;
	v[2]=0;
	v[3]=0;
	v[4]=0;
	v[5]=0;
	v[6]=0;
	v[7]=0;
	v[8]=1;
	v[9]=0;
	v[10]=0;
	v[11]=0;
	v[12]=0;
	in.push_back(v);

	v[0]=0;
	v[1]=0;
	v[2]=0;
	v[3]=1;
	v[4]=0;
	v[5]=0;
	v[6]=1;
	v[7]=0;
	v[8]=11;
	v[9]=1;
	v[10]=1;
	v[11]=0;
	v[12]=0;
	in.push_back(v);

	v[0]=0;
	v[1]=0;
	v[2]=0;
	v[3]=1;
	v[4]=1;
	v[5]=1;
	v[6]=1;
	v[7]=0;
	v[8]=1;
	v[9]=1;
	v[10]=1;
	v[11]=0;
	v[12]=0;
	in.push_back(v);


	v[0]=0;
	v[1]=0;
	v[2]=0;
	v[3]=1;
	v[4]=1;
	v[5]=1;
	v[6]=1;
	v[7]=0;
	v[8]=1;
	v[9]=1;
	v[10]=1;
	v[11]=0;
	v[12]=0;
	in.push_back(v);
	
	vector<int> ret;
	khun(in,ret);

	cout<<"in : "<<endl;
	for(int i=0; i<5; i++){
		for(int j=0; j<12; j++)
			cout<<in[i][j]<<" ";
		cout<<endl;
	}

	cout<<"ret : ";
	for(int i=0; i<5; i++)
		cout<<ret[i]<<" ";
	cout<<endl;
}









