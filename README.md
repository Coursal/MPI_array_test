# MPI_array_test
A C-MPI parallel program that manipulates (by adding or multiplying) some 1D-2D arrays, all chosen by the user through a simple text menu.

Arrays to work with:

1. A[1xN]
2. B[Nx1]
3. C[NxN]
4. D[NxN]

Operations between the arrays:

1. C+D
2. C*B
3. A*B
4. B*A

Steps:

1. input of the main dimension N for the arrays
2. initialization of all 4 arrays
3. choice of operation between arrays through text menu

Limitations:

1. only MPI_Bcast()/MPI_Reduce()/MPI_Scatter()/MPI_Gather() functions used, except the first operation which MPI_Scatterv()/MPI_Gatherv() functions are used
2. number of elements N of all arrays for all of the operations except the first one must be evenly divided by the number of the tasks

Guide to compile and run:

1. download the .tar file from here: ftp://ftp.mcs.anl.gov/pub/mpi/old/mpich-1.2.4.tar.gz
2. sudo apt-get install gcc
3. sudo apt-get install libmpich2-dev
4. mpicc -o mpi_array mpi_array.c
5. mpiexec -n "number_of_tasks" ./mpi_array

Tested on Lubuntu 12.04 (Precise Pangolin) on a virtual machine.