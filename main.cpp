#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "mpi.h"
using namespace std;

int main(int argc, char *argv[])
{
	vector<int> arr{};
	//read data from file
	ifstream file;
	file.open("example.txt");
	while (!file.eof())
	{
		int t;
		file >> t;
		arr.push_back(t);
	}
	file.close();

	//test
	sort(begin(arr), end(arr));

	int myid, numprocs;
	MPI::Init(argc, argv);
	numprocs = MPI::COMM_WORLD.Get_size();
	myid = MPI::COMM_WORLD.Get_rank();

	//test print
	for (auto &e : arr)
		cout << e << " ";
	cout << endl
		 << "Hello world" << endl;

	MPI::Finalize();
	return 0;
}
