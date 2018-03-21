#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "mpi.h"
using namespace std;

int main(int argc, char *argv[])
{
	//PHASE I
	vector<int> arr{};
	int myid, numprocs;
	MPI::Init(argc, argv);
	numprocs = MPI::COMM_WORLD.Get_size();
	myid = MPI::COMM_WORLD.Get_rank();

	if (myid == 0)
	{ //read data from file
		ifstream ifile;
		ifile.open("example.txt");
		while (!ifile.eof())
		{
			int t;
			ifile >> t;
			arr.push_back(t);
		}
		ifile.close();
	}

	//PHASE II

	//test
	//sort(begin(arr), end(arr));

	//ENDING
	//write to file
	ofstream ofile;
	ofile.open("result.txt");
	for (auto &e : arr)
		ofile << e << " ";
	ofile.close();

	MPI::Finalize();
	return 0;
}
