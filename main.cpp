#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <cmath>

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
	if (myid == 0)
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
	for (int i = 0; i < numprocs; i++)
	{
		if (i == numprocs - 1)
			local_partition_size.push_back(unsorted_data_size - i * par_size);
		else
			local_partition_size.push_back(par_size);
		local_partition_starts.push_back(i * par_size);
	}
	vector<int> data_partition(local_partition_size[myid]); // vector to receive data

	// PHASE II
	MPI::COMM_WORLD.Scatterv(unsorted_data.data(), &local_partition_size.data()[myid], &local_partition_starts.data()[myid], MPI::INT, data_partition.data(), local_partition_size.data()[myid], MPI::INT, 0);
	std::sort(begin(data_partition), end(data_partition));
	vector<int> local_samples{};
	for (int i = 0; i < numprocs; i++)
		local_samples.push_back(data_partition[static_cast<int>(i * unsorted_data_size / pow(numprocs, 2))]);

	//OK - DELETE AT THE END
	// stringstream ss;
	// ss << "result_" << myid << ".txt";
	// ofstream test_file;
	// test_file.open(ss.str());
	// for (auto &e : local_samples)
	// 	test_file << e << " ";
	// test_file.close();

	//PHASE III
	vector<int> pivots(numprocs * numprocs); //p elements from p processes
	vector<int> pivots_bcast{};
	MPI::COMM_WORLD.Gather(local_samples.data(), numprocs, MPI_INT, &pivots.data()[myid * numprocs], numprocs, MPI_INT, 0);
	if (myid == 0)
	{
		sort(begin(pivots), end(pivots));
		for (int i = numprocs; i < pivots.size(); i += numprocs)
			pivots_bcast.push_back(pivots[i]);
		// for (auto &e : pivots_bcast)
		// 	cout << e << " ";
	}
	else
		pivots_bcast.resize(numprocs - 1);

	MPI::COMM_WORLD.Bcast(pivots_bcast.data(), pivots_bcast.size(), MPI::INT, 0);

	//PHASE IV
	vector<int> iters; //lengths of subarrays in process
	auto tmp = begin(data_partition);
	for (auto &e : pivots_bcast)
	{
		auto t = upper_bound(tmp, end(data_partition), e - 1); //finds iterator greater than val
		iters.push_back(t - tmp);
		tmp = t;
	}
	iters.push_back(end(data_partition) - tmp);

	vector<int> ind_move(numprocs * numprocs);
	MPI::COMM_WORLD.Gather(iters.data(), numprocs, MPI_INT, &ind_move.data()[myid * numprocs], numprocs, MPI::INT, 0); //gather subarray sizes
	MPI::COMM_WORLD.Gatherv(data_partition.data(), local_partition_size.data()[myid], MPI_INT,
							unsorted_data.data(), &local_partition_size.data()[myid], &local_partition_starts.data()[myid], MPI_INT, 0); //gathers all arrays

	vector<int> final_local_sizes{};
	vector<vector<int>> arr_to_send{};
	if (myid == 0)
	{
		arr_to_send.resize(numprocs);
		for (int i = 0; i < numprocs; i++)	 //process nr
			for (int j = 0; j < numprocs; j++) //nr accessed proccess
			{
				int start_index = j * numprocs + i;
				int start_data = accumulate(begin(ind_move), begin(ind_move) + start_index, 0);
				for_each(begin(unsorted_data) + start_data, begin(unsorted_data) + start_data + ind_move[start_index], [&](int e) {
					arr_to_send[i].push_back(e);
				});
			}
		for (auto &e : arr_to_send)
			final_local_sizes.push_back(e.size());
	}
	else
		final_local_sizes.resize(numprocs);

	// PHASE V
	MPI::COMM_WORLD.Bcast(final_local_sizes.data(), numprocs, MPI_INT, 0);
	data_partition.resize(final_local_sizes[myid]);
	if (myid == 0) //send data to local arrays
	{
		data_partition.swap(arr_to_send[0]);
		for (int i = 1; i < numprocs; i++)
			MPI::COMM_WORLD.Send(arr_to_send[i].data(), final_local_sizes[i], MPI_INT, i, i);
	}
	else
		MPI::COMM_WORLD.Recv(data_partition.data(), final_local_sizes[myid], MPI_INT, 0, myid);

	std::sort(begin(data_partition), end(data_partition));

	// stringstream ss;
	// ss << "result_" << myid << ".txt";
	// ofstream test_file;
	// test_file.open(ss.str());
	// for (auto &e : data_partition)
	// 	test_file << e << " ";
	// test_file.close();

	// PHASE VI
	if (myid == 0)
	{
		vector<int> final_local_starts{0};
		int starts = 0;
		for (auto &e : final_local_sizes)
		{
			starts += e;
			final_local_starts.push_back(starts);
		}
		MPI::COMM_WORLD.Gatherv(data_partition.data(), final_local_sizes[myid], MPI_INT,
								unsorted_data.data(), &final_local_sizes.data()[myid], &final_local_starts.data()[myid], MPI_INT, 0); //again gathers all arrays
	}
	else
		MPI::COMM_WORLD.Gatherv(data_partition.data(), final_local_sizes[myid], MPI_INT,
								nullptr, nullptr, nullptr, MPI_INT, 0);

	//ENDING
	if (myid == 0)
	{
		ofstream ofile;
		ofile.open("result.txt");
		for (auto &e : unsorted_data)
			ofile << e << " ";
		ofile.close();
	}

	if (myid == 0)
		cout << "Is it sorted: " << is_sorted(begin(unsorted_data), end(unsorted_data)) << endl;
	MPI::Finalize();
	return 0;
}
