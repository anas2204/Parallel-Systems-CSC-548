/*
Group info:
hkhetaw Harsh Khetawat
asiddiq Anas Siddiqui
rkrish11 Rahul Krishna
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

/* The number of grid points */
#define NGRID 100000
/* first grid point */
#define XI 1.0
/* last grid point */
#define XF 100.0
/* 1 for using MPI_Reduce(), 0 for manual reduce */
#define MPIREDUCE 0
/* 1 for non-blocking send/receive, 0 for blocking calls */
#define NONBLOCKING 1

/* floating point precision type definitions */
typedef double FP_PREC;

/* function declarations */
FP_PREC fn(FP_PREC);
FP_PREC dfn(FP_PREC); 
FP_PREC ifn(FP_PREC, FP_PREC);
void print_error_data(int np, FP_PREC, FP_PREC, FP_PREC*, FP_PREC*, FP_PREC);

//To calculate the ideal min/max range for each process.
void calcMinMax(int numproc, int rank, int *imin, int *imax)
{
	*imin = 1 + (rank * (NGRID/numproc));
	int extra = NGRID%numproc;
	*imin += extra<rank?extra:rank;
	if(rank == numproc - 1)
		*imax = NGRID;
	else
	{
		*imax = (rank+1) * (NGRID/numproc);
		*imax += extra<rank+1?extra:rank+1;
	}
}

int main (int argc, char *argv[])
{
	int   numproc, rank, len,i;
	char  hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(hostname, &len);
	MPI_Request *requestList, request;

    FP_PREC     *yc, *dyc, *derr, *fullerr;
	FP_PREC     *xc, dx, intg, davg_err, dstd_dev, intg_err;
	FP_PREC globalSum = 0.0;

    // Real grid indices
    int         imin, imax;  
	calcMinMax(numproc, rank, &imin, &imax);
	
	
	int range = imax - imin + 1;
	
	xc  =   (FP_PREC*) malloc((range + 2) * sizeof(FP_PREC));
	yc  =   (FP_PREC*) malloc((range + 2) * sizeof(FP_PREC));
	dyc  =   (FP_PREC*) malloc((range + 2) * sizeof(FP_PREC));
	dx = (XF - XI)/(double)NGRID;
    for (i = 1; i <= range ; i++)
    {
		xc[i] = XI + dx * (imin + i - 2);
    }
	
    xc[0] = xc[1] - dx;
    xc[range + 1] = xc[range] + dx;
	
    for( i = 1; i <= range; i++ )
    {
    	yc[i] = fn(xc[i]);
    }
	
	yc[0] = fn(xc[0]);
	yc[range + 1] = fn(xc[range + 1]);
	
    for (i = 1; i <= range; i++)
    {
    	dyc[i] = (yc[i + 1] - yc[i - 1])/(2.0 * dx);
    }
	
    intg = 0.0;
    for (i = 1; i <= range; i++)
    {
		intg += 0.5 * (xc[i + 1] - xc[i]) * (yc[i + 1] + yc[i]);
    }
	
    // Calculate the final sum of the integration of all the parts.
	if(MPIREDUCE)
	{
		MPI_Reduce(&intg, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	else
	{
		if(rank == 0)
		{
			globalSum += intg;
			for(i=1; i<numproc; i++)
			{
				double partSum;
				MPI_Recv(&partSum, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				globalSum += partSum;
			}
		}
		else
		{
			MPI_Send(&intg, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		}
	}
	
    // Compute the error, average error of the derivatives.
    derr = (FP_PREC*)malloc((range + 2) * sizeof(FP_PREC));

    // Compute the errors.
    for(i = 1; i <= range; i++)
    {
    	derr[i] = fabs((dyc[i] - dfn(xc[i]))/dfn(xc[i]));
    }
	
	derr[0] = derr[range + 1] = 0.0;
	
	if(rank == 0)
	{
		fullerr = (FP_PREC *)malloc(sizeof(FP_PREC)*NGRID);
		for(i = 0;i<range;i++)
		{
			fullerr[i] = derr[i+1];
		}
		if(NONBLOCKING)
		{
			requestList =(MPI_Request*)malloc((numproc-1)*sizeof(MPI_Request));
		}
		for(i = 1; i<numproc; i++)
		{
		    int rmin, rmax;  
			calcMinMax(numproc, i, &rmin, &rmax);
			if(NONBLOCKING)
			{
				MPI_Irecv(fullerr+rmin-1, rmax-rmin+1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &(requestList[i-1]));
			}
			else
			{
				MPI_Recv(fullerr+rmin-1, rmax-rmin+1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}
		if(NONBLOCKING)
		{
			MPI_Waitall(numproc-1, requestList, MPI_STATUS_IGNORE);
		}
		double sum = 0.0;
		for(i=0; i<NGRID; i++)
		{
			sum+=fullerr[i];
		}
		davg_err = sum/(FP_PREC)NGRID;
	    dstd_dev = 0.0;
	    for(i = 0; i< NGRID; i++)
	    {
	    	dstd_dev += pow(fullerr[i] - davg_err, 2);
	    }
	    dstd_dev = sqrt(dstd_dev/(FP_PREC)NGRID);
  
	    intg_err = fabs((ifn(XI, XF) - globalSum)/ifn(XI, XF));
		printf("Integration: %lf\n", globalSum);
  		print_error_data(NGRID, davg_err, dstd_dev, &xc[1], fullerr, intg_err);
	}
	else
	{
		if(NONBLOCKING)
		{
			MPI_Isend(derr+1, imax-imin+1, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD, &request);
		}
		else
		{
			MPI_Send(derr+1, imax-imin+1, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);
		}
	}
	
	MPI_Finalize();
}

// Writes an err.dat file with average error, standard deviation of error, and error at each point.
void print_error_data(int np, FP_PREC avgerr, FP_PREC stdd, FP_PREC *x, FP_PREC *err, FP_PREC ierr)
{
	int   i;
	FILE *fp = fopen("err.dat", "w");
	double dx = x[1]-x[0];
	fprintf(fp, "%e\n%e\n%e\n", avgerr, stdd, ierr);
	for(i = 0; i < np; i++)
	{
		fprintf(fp, "%e %e \n", XI + i * dx, err[i]);
	}
	fclose(fp);
}

