#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

void* allocate(int dataLength)
{
	return (malloc(dataLength));
}

double calcMean(double values[11])
{
	int i;
	double mean=0.0;

	//Disregarding 1st value
	for(i=1; i<=10; i++)
		mean += values[i];

	return (mean/10.0);
}

double calcDeviation(double values[11], double mean)
{
	int i;
	double deviation = 0.0;

	//Disregarding 1st value
	for(i=1; i<=10; i++)
		deviation += ((mean - values[i]) * (mean - values[i]));

	return (sqrt(deviation));
}

int main (int argc, char *argv[])
{
	/* initialize MPI */
	MPI_Init(&argc, &argv);

	/* process information */
	int   numproc, rank,i,j,lastEven;
	double timeDiff;
	void *buffer;

	MPI_Status status;

	/* get the number of procs in the comm */
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);

	/* get my rank in the comm */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 

	int dataLength = 32,token;
	double node_avg, node_dev;

	lastEven = numproc - 2;

	//Main Loop start -> Process Pairing is done as (0,1),(2,3),(3,4) and so on.

	while(dataLength <= 2097152)			//32 Bytes to 2MB
	{
		buffer = allocate (dataLength);		
		token = 0;		//Token that gets passed between pairs

		//All Even numbered Processes
		if(rank %2 == 0)
		{
			//Array to store times of different runs
			double timeValues[11];

			if(rank == 0)
			{
				if(dataLength > 32)		//Except 1st run, 0th Process waits for Last Even process to start again
				{
					MPI_Recv(&token,1,MPI_INT,lastEven,20,MPI_COMM_WORLD,&status);
				}

				printf("\n%d",dataLength);
				fflush(stdout);
			}

			//All subsequent Even numbered Processes are waiting for the Token (2 waits for 0, 4 waits for 2, and so on)
			else
			{
				MPI_Recv(&token,1,MPI_INT,rank-2,rank,MPI_COMM_WORLD,&status);
			}

			//Process Starts message passing here. 11 Runs, will disregard the 1st run for calculating Mean and Std-Deviation
			for(j=0; j<=10; j++)
			{
				timeDiff = MPI_Wtime();
				MPI_Send (buffer,dataLength,MPI_BYTE,rank+1,rank,MPI_COMM_WORLD);
				MPI_Recv (buffer,dataLength,MPI_BYTE,rank+1,rank+1,MPI_COMM_WORLD,&status);

				//Calculating Difference in time and Storing
				timeDiff = MPI_Wtime() - timeDiff;

				timeValues[j] = timeDiff;
			}

			//Calculating Mean and Std-Deviation for 10 runs for a single pair
			node_avg = calcMean(timeValues);
			node_dev = calcDeviation(timeValues,node_avg);
				
			//Rank 0 prints its values, Other Even processes SEND to Process 0 for Printing
			if(rank == 0)
			{
				printf(" %lf %lf",node_avg,node_dev);
				fflush (stdout);
			}
			else
			{
				MPI_Send(&node_avg,1,MPI_DOUBLE,0,rank,MPI_COMM_WORLD);
				MPI_Send(&node_dev,1,MPI_DOUBLE,0,rank,MPI_COMM_WORLD);
			}

			//Informing Next Process Pair waiting after 11 runs: 2 scenarios			

			//1 -> Process 0 informing 2, 2 informing 4 and so on.
			if((rank+2)<numproc)
			{
				token = rank;
				MPI_Send(&token,1,MPI_INT,rank+2,rank+2,MPI_COMM_WORLD);
			}
			//2 -> Process 6 informing Process 0 in (0-7) OR Process 14 informing 0 (0-15) TO start Again with twice the data
			else if(dataLength < 2097152)
			{
				token = rank;
				MPI_Send(&token,1,MPI_INT,0,20,MPI_COMM_WORLD);
			}

			//This Gathers Avg and Deviation from Other Even Numbered Processes for a Single Run
			if(rank == 0)
			{
				int k;
				double tempMean, tempDev;

				for(k=1; k<(numproc/2); k++)
				{
					MPI_Recv(&tempMean,1,MPI_DOUBLE,(k*2),(k*2),MPI_COMM_WORLD,&status);
					MPI_Recv(&tempDev,1,MPI_DOUBLE,(k*2),(k*2),MPI_COMM_WORLD,&status);

					printf("% lf %lf",tempMean,tempDev);
					fflush (stdout);
				}
			}
		}

		//All Odd numbered processes
		else				
		{	 
			for(j=0; j<=10; j++)
			{
				MPI_Recv (buffer,dataLength,MPI_BYTE,rank-1,rank-1,MPI_COMM_WORLD,&status);	  
				MPI_Send (buffer,dataLength,MPI_BYTE,rank-1,rank,MPI_COMM_WORLD);
			}
		}

		free(buffer);
		dataLength *= 2;
	}

	MPI_Finalize();
}
