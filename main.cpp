#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "mpi.h"
using namespace std;

int main(int argc, char *argv[])
{
	// PHASE I
	vector<int> arr{};
        int myid, numprocs;
	MPI::Init(argc, argv);
	numprocs = MPI::COMM_WORLD.Get_size();
	myid = MPI::COMM_WORLD.Get_rank();
        int arr_size;
	if(myid == 0)
	{ // read data from file
		ifstream ifile;
		ifile.open("example.txt");
		while (!ifile.eof())
		{
			int t;
			ifile >> t;
			arr.push_back(t);
		}
		ifile.close();
                arr_size = arr.size();
	}
        MPI::COMM_WORLD.Bcast(&arr_size, 1, MPI_INT, 0);
	vector<int> local_size{};   // size of local vectors
	vector<int> local_starts{}; // starts of local vectors
	int par_size = arr_size / numprocs;
	for(int i = 0; i < numprocs; i++)
	{
		if(i == numprocs - 1)
			local_size.push_back(arr_size - i * par_size);
		else
			local_size.push_back(par_size);
		local_starts.push_back(i * par_size);
	}
	// for(auto &e : local_size) // test partitioning
	// 	cout<<e<<endl;
	// Nie wiem czy potrzebne
	vector<int> results{}; // vector to receive data
	/*int res_size;
	for(int i = 0; i < numprocs; i++)
	{
		vector<int> el{};
		results.push_back(el);
	}
*/
	// PHASE II
	results.resize(local_size[myid]);
	if(myid == 0)
		MPI::COMM_WORLD.Scatterv(&arr[0], &local_size[myid], &local_starts[myid], MPI::INT, &results[0], local_size[myid], MPI::INT, 0);
	else
		MPI::COMM_WORLD.Scatterv(&arr[0], &local_size[myid], &local_starts[myid], MPI::INT, &results[0], local_size[myid], MPI::INT, 0);
	
	std::sort(begin(results), end(results));

        //vector<int> pivots{};
        //for(int i=0; i<numprocs; i++)
          //  samples.push_back(arr[i*local_size[myid]/numprocs]);
//cout << local_size[myid] << "\t" << numprocs << endl;
        //cout << samples[0] << endl;
	// ENDING
	// write to file
	ofstream ofile;
	ofile.open("result.txt");
	for(auto &e : arr)
		ofile << e << " ";
	ofile.close();

	MPI::Finalize();
	return 0;
}
