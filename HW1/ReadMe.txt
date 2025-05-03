README - Parallel Computing HW1
================================

This project contains two parallel implementations of a grid computation problem using MPI:
1. static.c  – Static Task Pool version
2. dynamic.c – Dynamic Task Pool version

Files:
------
- static.c   → Implements static task division with one MPI_Send per slave
- dynamic.c  → Implements dynamic task distribution using a task queue
- results.doc → Execution time measurements and explanation
- README.TXT → This file

Compilation Instructions:
-------------------------
Use the following command to compile each file:

    mpicc static.c -o static -lm
    mpicc dynamic.c -o dynamic -lm

Execution Instructions:
-----------------------
To run each solution with N processes (e.g., 5 processes: 1 master + 4 slaves):

    mpiexec -np 5 ./static
    mpiexec -np 5 ./dynamic

Note:
-----
- `--oversubscribe` is required if N > number of physical CPU cores.
- Execution time will be printed at the end of the run.
- Run each configuration (2, 4, 10 slaves) multiple times and average results for reporting.

Authors:
-------
[Vladislav Pavlyuk - 332294891]
[Ronen Shershnev - 322217175]