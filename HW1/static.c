#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define HEAVY  100000
#define SIZE   30

double heavy(int x, int y) {
    int i, loop;
    double sum = 0;

    if ((x == 3 && y == 3) || (x == 3 && y == 5) || (x == 3 && y == 7) || (x == 20 && y == 10))
        loop = 200;
    else
        loop = 1;

    for (i = 1; i < loop * HEAVY; i++)
        sum += cos(exp(cos((double)i / HEAVY)));

    return sum;
}

// Calculate the partial sum for the assigned x range
double calculate_partial_sum(int x_start, int x_end, int size) {
    double partial_sum = 0.0;
    for (int x = x_start; x <= x_end; x++) {
        for (int y = 0; y < size; y++) {
            partial_sum += heavy(x, y);
        }
    }
    return partial_sum;
}

int main(int argc, char* argv[]) {
    int rank, size;
    double total_sum = 0.0;
    int num_workers;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    num_workers = size - 1; // Exclude master

    double start_time, end_time;
    if (rank == 0) {
        start_time = MPI_Wtime();

        int rows_per_worker = SIZE / num_workers;
        int extra_rows = SIZE % num_workers; // if SIZE not divisible evenly

        int x_start = 0;
        for (int worker = 1; worker <= num_workers; worker++) {
            int rows = rows_per_worker + (worker <= extra_rows ? 1 : 0);
            int x_end = x_start + rows - 1;

            int data[2] = {x_start, x_end};
            MPI_Send(data, 2, MPI_INT, worker, 0, MPI_COMM_WORLD);

            x_start = x_end + 1;
        }

        // Master does not compute any heavy() here.

        for (int worker = 1; worker <= num_workers; worker++) {
            double partial_sum;
            MPI_Recv(&partial_sum, 1, MPI_DOUBLE, worker, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_sum += partial_sum;
        }

        end_time = MPI_Wtime();
        printf("Final answer = %e\n", total_sum);
        printf("Execution time = %f seconds\n", end_time - start_time);

    } else {
        int data[2];
        MPI_Recv(data, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int x_start = data[0];
        int x_end = data[1];

        double partial_sum = calculate_partial_sum(x_start, x_end, SIZE);
        MPI_Send(&partial_sum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}