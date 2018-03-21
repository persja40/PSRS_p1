#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "mpi.h"
using namespace std;

int main(int argc, char *argv[])
{
	// PHASE I
	vector<int> unsorted_data{};
        int myid, numprocs;
	MPI::Init(argc, argv);
	numprocs = MPI::COMM_WORLD.Get_size();
	myid = MPI::COMM_WORLD.Get_rank();
        int unsorted_data_size;
	if(myid == 0)
	{ // read data from file
		ifstream ifile;
		ifile.open("example.txt");
		while (!ifile.eof())
		{
			int t;
			ifile >> t;
			unsorted_data.push_back(t);
		}
		ifile.close();
                unsorted_data_size = unsorted_data.size();
	}
        MPI::COMM_WORLD.Bcast(&unsorted_data_size, 1, MPI_INT, 0);
	vector<int> local_partition_size{};   // size of local vectors
	vector<int> local_partition_starts{}; // starts of local vectors
	int par_size = unsorted_data_size / numprocs;
	for(int i = 0; i < numprocs; i++)
	{
		if(i == numprocs - 1)
			local_partition_size.push_back(unsorted_data_size - i * par_size);
		else
			local_partition_size.push_back(par_size);
		local_partition_starts.push_back(i * par_size);
	}
	vector<int> data_partition{}; // vector to receive data

	// PHASE II
	data_partition.resize(local_partition_size[myid]);
	if(myid == 0)
		MPI::COMM_WORLD.Scatterv(&unsorted_data[0], &local_partition_size[myid], &local_partition_starts[myid], MPI::INT, &data_partition[0], local_partition_size[myid], MPI::INT, 0);
	else
		MPI::COMM_WORLD.Scatterv(&unsorted_data[0], &local_partition_size[myid], &local_partition_starts[myid], MPI::INT, &data_partition[0], local_partition_size[myid], MPI::INT, 0);
	
	std::sort(begin(data_partition), end(data_partition));

        stringstream ss;
        ss << "result_" << myid << ".txt";
	ofstream test_file;
	test_file.open(ss.str());
	for(auto &e : unsorted_data)
		test_file << e << " ";
	test_file.close();

        //vector<int> pivots{};
        //for(int i=0; i<numprocs; i++)
          //  samples.push_back(unsorted_data[i*local_partition_size[myid]/numprocs]);
//cout << local_partition_size[myid] << "\t" << numprocs << endl;
        //cout << samples[0] << endl;
	// ENDING
	// write to file
	ofstream ofile;
	ofile.open("result.txt");
	for(auto &e : unsorted_data)
		ofile << e << " ";
	ofile.close();

	MPI::Finalize();
	return 0;
}
