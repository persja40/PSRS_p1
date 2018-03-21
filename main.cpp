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
	vector<int> local_size{}; //size of local vectors
	int par_size = arr.size() / numprocs;
	for (int i = 0; i < numprocs; i++)
		if (i == numprocs - 1)
			local_size.push_back(arr.size() - i * par_size);
		else
			local_size.push_back(par_size);
	// for(auto &e : local_size) //test partitioning
	// 	cout<<e<<endl;
	vector<vector<int>> results{}; // vector to receive data
	for (int i = 0; i < numprocs; i++)
	{
		vector<int> el{};
		results.push_back(el);
	}

	//PHASE II DO POPRAWY
	if (myid == 0)
	{
		MPI::COMM_WORLD.Scatterv(&arr[0], &local_size[0], ,MPI::INT,
								 //MPI_IN_PLACE, &results[myid], MPI::INT, 0);
	}
	else
	{
		// MPI::COMM_WORLD.Scatterv(&arr[myid * par_size], &local_size[myid], MPI::INT,
		// 						 &arr[0], &results[myid], MPI::INT, 0);
	}
	sort(begin(arr), end(arr));

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
