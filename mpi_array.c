#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

void init_arrays(int A[], int B[], int C[], int D[], int n)
{
	int i, j;
	
	printf("\n");
	printf("A[] matrix:\n\n");			//scan the numbers the user gave you for A[]		
	for(i=0;i<1;i++)				
	{
		for(j=0;j<n;j++)
		{
			printf("A[%d][%d]: ", i, j);
			scanf("%d", &A[j]);
		}
		printf("\n");
	}	
	
	printf("B[] matrix:\n\n");			//scan the numbers the user gave you for B[]		
	for(i=0;i<n;i++)				
	{
		for(j=0;j<1;j++)
		{
			printf("B[%d][%d]: ", i, j);
			scanf("%d", &B[i]);
		}
	}	
	
	printf("\n");
	printf("C[] matrix:\n\n");			//scan the numbers the user gave you for C[]		
	for(i=0;i<n;i++)				
	{
		for(j=0;j<n;j++)
		{
			printf("C[%d][%d]: ", i, j);
			scanf("%d", &C[i*n+j]);
		}
		printf("\n");
	}	
	
	printf("D[] matrix:\n\n");			//scan the numbers the user gave you for D[]		
	for(i=0;i<n;i++)				
	{
		for(j=0;j<n;j++)
		{
			printf("D[%d][%d]: ", i, j);
			scanf("%d", &D[i*n+j]);
		}
		printf("\n");
	}	
	
}

int comp_menu(int n)
{
	int choice;
	
	printf("\n\n\n");
	printf("^^^^^^^^^^^^^^^MENU^^^^^^^^^^^^^^^");
	printf("\n\n");
	printf("\t1. C[%dx%d] + D[%dx%d]\n",n, n, n, n);
	printf("\t2. C[%dx%d] * B[%dx1]\n", n, n, n);
	printf("\t3. A[1x%d] * B[%dx1]\n", n, n);
	printf("\t4. B[%dx1] * A[1x%d]\n", n, n);
	printf("\n");
	printf("Choice: ");
	scanf("%d", &choice);
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
	printf("\n\n\n");
	
	return choice;
}


int main(int argc, char** argv)
{
	int cpu_rank;				//name of the cpu/task
	int n;						//the main dimension for the A[], B[], C[], D[] matrices 
	int p;						//number of tasks
	int i;
	int j;
	int k;

	int choice;
	int flag;					
	
	int *A, *B, *C, *D;

	MPI_Init(&argc, &argv);
		MPI_Comm_rank(MPI_COMM_WORLD, &cpu_rank);
		MPI_Comm_size(MPI_COMM_WORLD, &p);
		
		
		if(cpu_rank==0)
		{
			printf("Give me the main dimension N for the forthcoming arrays: ");
			scanf("%d", &n);		
		}
		
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
		A=(int *)malloc(n*sizeof(int));		//even though these are 2d matrices, 
		B=(int *)malloc(n*sizeof(int));		//it's easier to handle it as a 1d matrix
		C=(int *)malloc((n*n)*sizeof(int));
		D=(int *)malloc((n*n)*sizeof(int));
		
		if(cpu_rank==0)
		{
			init_arrays(A, B, C, D, n);		//initialize the arrays
		}
		
		do
		{
			
			if(cpu_rank==0)
			{
				choice=comp_menu(n);		//choose an operation from the menu
			}
			
			
			MPI_Bcast(&choice, 1, MPI_INT, 0, MPI_COMM_WORLD);
			
			if(choice==1)
			{
				int *each_nums;				//array of the numbers each task will handle
				int *disps;					//array of the offsets of the data divided to the tasks
				int remainder=(n*n)%p;		//the remaining number of records that the last
				int sum=0;
				
				disps=malloc(p*sizeof(int));
				each_nums=malloc(p*sizeof(int));
				
				int ADD1[n*n];				//array with the result of the addition
				int ADD1_local[n*n];		//array with the local result of the addition by each task
				
				int C_local[n*n];			//arrays with the local C[]/D[] numbers of each task
				int D_local[n*n];
				
				for(i=0;i<p;i++)			//calculation of number of elements each task will handle and offset of them
				{
					each_nums[i]=(n*n)/p;
					if(remainder>0)
					{
						each_nums[i]++;
						remainder--;
					}
					
					disps[i]=sum;
					sum+=each_nums[i];
				}
				
				MPI_Scatterv(C, each_nums, disps, MPI_INT, C_local, each_nums[cpu_rank], MPI_INT, 0, MPI_COMM_WORLD);		//scatter pieces of C[] to all tasks
				MPI_Scatterv(D, each_nums, disps, MPI_INT, D_local, each_nums[cpu_rank], MPI_INT, 0, MPI_COMM_WORLD);		//scatter pieces of D[] to all tasks
				
	
				for(i=0;i<each_nums[cpu_rank];i++)	//add each number of C[] to the number at D[] at the same position
					ADD1_local[i]=C_local[i]+D_local[i];
				
				 
				MPI_Gatherv(ADD1_local, each_nums[cpu_rank], MPI_INT, ADD1, each_nums, disps, MPI_INT, 0, MPI_COMM_WORLD);	//gather all the local results of each task to ADD1[]
				
				
				if(cpu_rank==0)						//if you are the cpu coordinator
				{			
					printf("\n\nADD1[] matrix:\n\n");		//print the results of C[]+D[]
					for(i=0;i<n;i++)
					{
						for(j=0;j<n;j++)
						{
							printf("ADD1[%d][%d]=%d \t", i, j, ADD1[i*n+j]);
						}
						printf("\n");
					}	
				}
				
			}
			else if(choice==2)
			{
				int num_of_numbers=n/p*n;	//number of numbers each task will handle
				int num_of_rows=n/p;		//dividing C[] by rows
			
				int MULT2[n];	
				int MULT2_local[n];	
			
				int C_local[n*n];
				
				MPI_Scatter(C, num_of_numbers, MPI_INT, C_local, num_of_numbers, MPI_INT, 0, MPI_COMM_WORLD);
				MPI_Bcast(&(B[0]), n, MPI_INT, 0, MPI_COMM_WORLD);
				
				for(i=0;i<num_of_rows;i++)			//multiplying the records of C[] and B[] and then add the products together
				{									//to a sum of each row
					MULT2_local[i]=0;
					for(j=0;j<n;j++)
						MULT2_local[i]+=C_local[i*n+j]*B[j];
				}	
					
				MPI_Gather(MULT2_local, num_of_rows, MPI_INT, MULT2, num_of_rows, MPI_INT, 0, MPI_COMM_WORLD);
				
				if(cpu_rank==0)						//if you are the cpu coordinator
				{			
					printf("\n\nMULT2[] matrix:\n\n");		//print the results of C[]*B[]
					for(i=0;i<n;i++)
					{
						for(j=0;j<1;j++)
						{
							printf("MULT2[%d][%d]=%d \t", i, j, MULT2[i]);
						}
						printf("\n");
					}	
				}
					
			}
			else if(choice==3)
			{
				int num_of_numbers=n/p;			//number of numbers each task will handle
			
				int A_local[n];
				int B_local[n];
			
				int MULT3=0;
	
				MPI_Scatter(A, num_of_numbers, MPI_INT, A_local, num_of_numbers, MPI_INT, 0, MPI_COMM_WORLD);		//send pieces of A to all tasks
				MPI_Scatter(B, num_of_numbers, MPI_INT, B_local, num_of_numbers, MPI_INT, 0, MPI_COMM_WORLD);		//send pieces of A to all tasks			
				
				int MULT3_local=0;
				
				for(i=0;i<num_of_numbers;i++)			//multiply the right numbers and then add the products together
				{
					MULT3_local+=A_local[i]*B_local[i];
				}
			
				MPI_Reduce(&MULT3_local, &MULT3, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);	//add all of the results from all tasks and save them to MULT3
			
				if(cpu_rank==0)			//if you are the cpu coordinator
				{
					printf("\n\nMULT3[] matrix:\n\n");		//print the results of A[]*B[]
					printf("MULT3[0]=%d", MULT3);
					printf("\n");
				}
			}
			else if(choice==4)
			{
				int num_of_numbers=n/p;					//number of numbers each task will handle
				int num_of_records=n*num_of_numbers;	//number of records each task will generate
				
				int B_local[n];
			
				int MULT4[n*n];
				int MULT4_local[num_of_records];
		
				MPI_Scatter(B, num_of_numbers, MPI_INT, B_local, num_of_numbers, MPI_INT, 0, MPI_COMM_WORLD);		//send pieces of A to all tasks			
				MPI_Bcast(A, n, MPI_INT, 0, MPI_COMM_WORLD);
			
				k=0;
				for(i=0;i<num_of_numbers;i++)
				{
					for(j=0;j<n;j++)
					{
						MULT4_local[k]=B_local[i]*A[j];
						k++;
					}
				
				}
			
				MPI_Gather(MULT4_local, num_of_records, MPI_INT, MULT4, num_of_records, MPI_INT, 0, MPI_COMM_WORLD);
			
				if(cpu_rank==0)						//if you are the cpu coordinator
				{			
					printf("\n\nMULT4[] matrix:\n\n");		//print the results of B[]*A[]
					for(i=0;i<n;i++)
					{
						for(j=0;j<n;j++)
						{
							printf("MULT4[%d][%d]=%d \t", i, j, MULT4[i*n+j]);
						}
						printf("\n");
					}	
				}
			}
			else
			{
				if(cpu_rank==0)
					printf("WRONG CHOICE\n");
			}
			
			if(cpu_rank==0)
			{
				printf("\n\nContinue? [1/0]: ");
				scanf("%d", &flag);	
				
				if(flag!=0 && flag!=1)
				{
					printf("WRONG ANSWER\nTERMINATION\n");
					exit(1);
				}
			}
			
			MPI_Bcast(&flag, 1, MPI_INT, 0, MPI_COMM_WORLD);	
		}
		while(flag);
		
	MPI_Finalize();
	return 0;
}