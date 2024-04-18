#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <random>
#include <mpi.h>

using namespace std;

void bubbleSort(vector<int>& arr) {
    bool isSorted = false;

    while (!isSorted) {
        isSorted = true;
        for (int i = 0; i < arr.size() - 1; i++) {
            if (arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
    }
}

void parallelbubbleSort(vector<int>& arr, int start, int end) {
    bool isSorted = false;

    while (!isSorted) {
        isSorted = true;
        for (int i = start; i < end - 1; i++) {
            if (i % 2 == 0 && arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
            else if (i % 2 != 0 && arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n;
    if (rank == 0) {
        cout << "Enter n: ";
        cin >> n;
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<int> arr(n);
    if (rank == 0) {
        int min = 0;
        int max = 100000;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dist(min, max);

        for (int i = 0; i < n; i++) {
            arr[i] = dist(gen);
        }
    }
    vector<int> arr_sequential = arr;

    vector<int> local_arr(n / size);
    MPI_Scatter(arr.data(), n / size, MPI_INT, local_arr.data(), n / size, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        auto start = chrono::high_resolution_clock::now();
        parallelbubbleSort(local_arr, 0, local_arr.size());
        vector<int> sorted_arr(n);
        MPI_Gather(local_arr.data(), local_arr.size(), MPI_INT, sorted_arr.data(), local_arr.size(), MPI_INT, 0, MPI_COMM_WORLD);
        parallelbubbleSort(sorted_arr, 0, sorted_arr.size());
        auto end = chrono::high_resolution_clock::now();

        auto duration_seconds = chrono::duration_cast<chrono::seconds>(end - start);
        auto duration_milliseconds = chrono::duration_cast<chrono::milliseconds>(end - start);
        cout << endl << "Parallel operating time: " << duration_seconds.count() << " seconds ";
        cout << duration_milliseconds.count() % 1000 << " milliseconds" << endl << endl;

        
        cout << "Parallel sorted array: ";
        for (int i = 0; i < min(40, (int)sorted_arr.size()); i++) {
            cout << sorted_arr[i] << " ";
        }
        cout << endl;


        auto start_sequential = chrono::high_resolution_clock::now();
        bubbleSort(arr_sequential);
        auto end_sequential = chrono::high_resolution_clock::now();

        auto duration_seconds_seq = chrono::duration_cast<chrono::seconds>(end_sequential - start_sequential);
        auto duration_milliseconds_seq = chrono::duration_cast<chrono::milliseconds>(end_sequential - start_sequential);
        cout << endl << "Sequential operating time: " << duration_seconds_seq.count() << " seconds ";
        cout << duration_milliseconds_seq.count() % 1000 << " milliseconds" << endl << endl;


        cout << "Sequential sorted array: ";
        for (int i = 0; i < min(40, (int)arr_sequential.size()); i++) {
            cout << arr_sequential[i] << " ";
        }
        cout << endl;;

    }
    else {
        MPI_Gather(local_arr.data(), local_arr.size(), MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
