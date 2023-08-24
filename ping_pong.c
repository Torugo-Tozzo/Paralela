/*                                                                      */
/* CS 4331: Parallel Programming                        Fall, 2009      */
/*                                                                      */
/* This MPI program computes does a "ping-pong" message passing		*/
/* performance measurement.						*/
/*                                                                      */
#include <stdio.h>
#include "mpi.h"

#define  MAX_LEN 1 << 18	/* maximum vector length	*/
#define  TRIALS  100		/* trials for each msg length	*/
#define  PROC_0  0		/* processor 0			*/
#define  B0_TYPE 176		/* message "types"		*/
#define  B1_TYPE 177

int main(argc,argv)
    int argc;
    char *argv[];

{
    int numprocs, p,		/* number of processors, proc index	*/
	myid,			/* this processor's "rank"		*/
	length,			/* vector length			*/
	i, t;			

    double b0[MAX_LEN], b1[MAX_LEN];	/* vectors			*/

    double start_time, end_time;	/* "wallclock" times		*/

    MPI_Status stat;		/* MPI structure containing return	*/
				/* codes for message passing operations */

    MPI_Request send_handle, recv_handle;	/* For nonblocking msgs	*/

    MPI_Init(&argc,&argv);			/* initialize MPI	*/

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);	/*how many processors?	*/
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);	/*which one am I?	*/

    if (myid == PROC_0)
	{
	srand48(0xFEEDFACE);

	/* generate processor 0's vector */

	for (i=0; i<MAX_LEN; ++i)
	    b0[i] = (double) drand48();
	}

    MPI_Barrier(MPI_COMM_WORLD);

    /* warmup, if necessary */
/*
    printf("# Start warmup.\n");

    for (length=1; length<=MAX_LEN; length*=2)
	if (myid == PROC_0)
	{
		MPI_Send(b0, length, MPI_DOUBLE, 1, B1_TYPE, MPI_COMM_WORLD);
		MPI_Recv(b1, length, MPI_DOUBLE, 1, B0_TYPE, MPI_COMM_WORLD, &stat);
	}
	else
	{
		MPI_Recv(b1, length, MPI_DOUBLE, 0, B1_TYPE, MPI_COMM_WORLD, &stat);
		MPI_Send(b0, length, MPI_DOUBLE, 0, B0_TYPE, MPI_COMM_WORLD);
	}
*/
    if (myid == PROC_0)
#ifdef BLOCKING
	printf("\nPing-pong measurements for blocking send/recv pairs.\n");
#else
	printf("\nPing-pong measurements for nonblocking send/recv pairs.\n");
#endif
    /* measure message passing speed for vectors of various lengths */

    for (length=1; length<=MAX_LEN; length*=2)
    {
	MPI_Barrier(MPI_COMM_WORLD);

	if (myid == PROC_0)
		start_time = MPI_Wtime();

	for (t=0; t<TRIALS; ++t)
	{
#ifdef BLOCKING
		if (myid == PROC_0)
		{
			MPI_Send(b0, length, MPI_DOUBLE, 1, B1_TYPE, MPI_COMM_WORLD);
			MPI_Recv(b1, length, MPI_DOUBLE, 1, B0_TYPE, MPI_COMM_WORLD, &stat);
		}
		else
		{
			MPI_Recv(b1, length, MPI_DOUBLE, 0, B1_TYPE, MPI_COMM_WORLD, &stat);
			MPI_Send(b0, length, MPI_DOUBLE, 0, B0_TYPE, MPI_COMM_WORLD);
		}
#else
		MPI_Isend(b0, length, MPI_DOUBLE, (myid+1)%numprocs, B0_TYPE, MPI_COMM_WORLD, &send_handle);
		MPI_Irecv(b1, length, MPI_DOUBLE, (myid+1)%numprocs, B0_TYPE, MPI_COMM_WORLD, &recv_handle);

		MPI_Wait(&send_handle, &stat);
		MPI_Wait(&recv_handle, &stat);
#endif
	}
	if (myid == PROC_0)
	    {
	    end_time = MPI_Wtime();

	    /* gnuplot ignores output lines that begin with #. */
	    printf("# Length = %d\tAverage time=%lf\n",
		length, (end_time - start_time)/(double)(2*TRIALS));
	    printf(" %d\t %lf\n",
		length, (end_time - start_time)/(double)(2*TRIALS));
	    }
    }

    MPI_Finalize();
}