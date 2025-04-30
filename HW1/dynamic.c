/**
 * dynamic.c - Dynamic Task Pool Implementation
 * 
 * This program implements a dynamic task pool approach to parallelize
 * the computation of the heavy() function over a 2D grid.
 * 
 * In dynamic task pool, master sends tasks to slaves on demand
 * and slaves request new tasks when they finish their current task.
 * This helps balance the load across processors more effectively,
 * especially when task execution times are unpredictable.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 #include <mpi.h>
 #include <time.h>
 
 #define HEAVY  100000
 #define SIZE   30
 
 #define TASK_TAG 1
 #define RESULT_TAG 2
 #define TERMINATE_TAG 3
 
 // Structure to represent a task
 typedef struct {
     int x;
     int y;
 } Task;
 
 // Structure to represent a result
 typedef struct {
     double result;
 } Result;
 
 /**
  * This function performs heavy computations, 
  * its run time depends on x and y values
  * DO NOT change the function
  */
 double heavy(int x, int y) {
     int i, loop;
     double sum = 0;
     
     if ((x == 3 && y == 3) || (x == 3 && y == 5) ||
         (x == 3 && y == 7) || (x == 20 && y == 10))
         loop = 200;
     else
         loop = 1;
     
     for (i = 1; i < loop * HEAVY; i++)
         sum += cos(exp(cos((double)i / HEAVY)));
     
     return sum;
 }
 
 /**
  * Function for master process to initialize and distribute tasks
  * dynamically to slave processes
  */
 void master_process(int num_slaves) {
     int size = SIZE;
     double final_answer = 0.0;
     int tasks_sent = 0;
     int tasks_received = 0;
     int total_tasks = size * size;
     int slave_id;
     MPI_Status status;
     
     Task task;
     Result result;
     
     // Time measurement
     double start_time = MPI_Wtime();
     
     // Initialize tasks
     int initial_tasks = num_slaves < total_tasks ? num_slaves : total_tasks;
     
     // Send initial tasks to all slaves
     for (int i = 0; i < initial_tasks; i++) {
         task.x = i / size;
         task.y = i % size;
         MPI_Send(&task, sizeof(Task), MPI_BYTE, i + 1, TASK_TAG, MPI_COMM_WORLD);
         tasks_sent++;
     }
     
     // Receive results and send more tasks as slaves finish
     while (tasks_received < total_tasks) {
         // Receive a result from any slave
         MPI_Recv(&result, sizeof(Result), MPI_BYTE, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);
         
         // Add result to the final answer
         final_answer += result.result;
         tasks_received++;
         
         // Get the slave ID
         slave_id = status.MPI_SOURCE;
         
         // Send new task if there are more tasks
         if (tasks_sent < total_tasks) {
             task.x = tasks_sent / size;
             task.y = tasks_sent % size;
             MPI_Send(&task, sizeof(Task), MPI_BYTE, slave_id, TASK_TAG, MPI_COMM_WORLD);
             tasks_sent++;
         } else {
             // No more tasks, send termination signal
             MPI_Send(NULL, 0, MPI_BYTE, slave_id, TERMINATE_TAG, MPI_COMM_WORLD);
         }
     }
     
     // Send termination signal to all remaining slaves
     for (int i = 1; i <= num_slaves; i++) {
         MPI_Send(NULL, 0, MPI_BYTE, i, TERMINATE_TAG, MPI_COMM_WORLD);
     }
     
     // Calculate elapsed time
     double end_time = MPI_Wtime();
     double execution_time = end_time - start_time;
     
     // Print results
     printf("Dynamic Task Pool with %d slaves\n", num_slaves);
     printf("answer = %e\n", final_answer);
     printf("Execution time: %.6f seconds\n", execution_time);
 }
 
 /**
  * Function for slave processes to receive tasks,
  * compute results, and send them back to the master
  */
 void slave_process(int rank) {
     MPI_Status status;
     Task task;
     Result result;
     
     while (1) {
         // Receive a task from the master
         MPI_Recv(&task, sizeof(Task), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
         
         // Check if termination signal is received
         if (status.MPI_TAG == TERMINATE_TAG) {
             break;
         }
         
         // Compute the result
         result.result = heavy(task.x, task.y);
         
         // Send the result back to the master
         MPI_Send(&result, sizeof(Result), MPI_BYTE, 0, RESULT_TAG, MPI_COMM_WORLD);
     }
 }
 
 /**
  * Sequential implementation of the computation
  * Used for verification and comparison
  */
 double run_sequential() {
     int x, y;
     int size = SIZE;
     double answer = 0;
     
     double start_time = MPI_Wtime();
     
     for (x = 0; x < size; x++)
         for (y = 0; y < size; y++)
             answer += heavy(x, y);
     
     double end_time = MPI_Wtime();
     double execution_time = end_time - start_time;
     
     printf("Sequential solution\n");
     printf("answer = %e\n", answer);
     printf("Execution time: %.6f seconds\n", execution_time);
     
     return answer;
 }
 
 /**
  * Main function
  */
 int main(int argc, char* argv[]) {
     int rank, size;
     
     // Initialize MPI
     MPI_Init(&argc, &argv);
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     MPI_Comm_size(MPI_COMM_WORLD, &size);
     
     if (size < 2) {
         if (rank == 0) {
             printf("Error: This program requires at least 2 processes (1 master + at least 1 slave)\n");
             printf("Running sequential version instead...\n");
             run_sequential();
         }
         MPI_Finalize();
         return 1;
     }
     
     // Number of slave processes
     int num_slaves = size - 1;
     
     if (rank == 0) {
         // Master process
         master_process(num_slaves);
     } else {
         // Slave process
         slave_process(rank);
     }
     
     MPI_Finalize();
     return 0;
 }