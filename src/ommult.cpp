#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#define DEFAULT_FN "res2_c.mat"
#ifdef DEBUG
#define DEFAULT_SIZE 1024
#else
#define DEFAULT_SIZE 3072
#endif


void minit(int size, double *A)
{
    time_t t;

    srand((unsigned)time(&t));

    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
#ifdef DEBUG
            A[i*size+j] = i*(j+1);
#else
            A[i*size+j] = rand() % 1000;
#endif
        }
    }
}


void mwrite(int size, double *A, char *fn)
{
    FILE *f = fopen(fn, "w+");

    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {
            fprintf(f, "%g\t", A[i*size+j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}


void mmult(int size, int nslices, double *A, double *B, double *C)
{
    for(int i=0; i<size/nslices; i++)
    {
        for(int j=0; j<size; j++)
        {
            double res = 0.0;

            for(int k=0; k<size; k++)
            {
                res += A[i*size+k]*B[k*size+j];
            }

            C[i*size+j] += res;
        }
    }
}


int main(int argc, char *argv[])
{
    int mr, nproc, size, slice;
    double *mat_a, *mat_b, *mat_c;
    char filename[32];
    MPI_Status st;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mr);	// my rank
    MPI_Comm_size(MPI_COMM_WORLD, &nproc); // number of processors

    if(argc > 3)
    {
        if(mr == 0)
        {
            printf("Usage: ./mmult2_c.exe SIZE FILENAME\n \
          \tSIZE: size of the matrix to compute (default is %d)\n \
          \tFILENAME: output matrix file name (default is %s)\n", DEFAULT_SIZE, DEFAULT_FN);
        }

        return 1;
    }
    else
    {
        if(argc > 1)
            size = atoi(argv[1]); // set size
        else
            size = DEFAULT_SIZE;

        if(argc == 3)
            strcpy(filename, argv[2]); // set filename
        else
            strcpy(filename, DEFAULT_FN);
    }

    if(size%nproc)
    {
        if(mr == 0)
            printf("Error: SIZE (%d) must be a multiple of number of processes (%d)\n", size, nproc);

        return 1;
    }

    slice = size*size/nproc; // set slice size in number of elements

    if(mr == 0)
        printf("%d: Size of the matrices: %dx%d\n", mr, size, size);

    if(mr == 0)
    {
        mat_a = (double*)malloc(size*size*sizeof(double));
        mat_b = (double*)malloc(size*size*sizeof(double));
        mat_c = (double*)malloc(size*size*sizeof(double));

        printf("%d: Initializing matrices...\n", mr);

        minit(size, mat_a);
        minit(size, mat_b);
        minit(size, mat_c);

        printf("%d: Sending matrices...\n", mr);

        for(int i=1; i<nproc; i++)
        {
            MPI_Send ( &mat_a[slice*i], slice, MPI_DOUBLE, i, i, MPI_COMM_WORLD );
            MPI_Send ( &mat_b[0], size*size, MPI_DOUBLE, i, 100+i, MPI_COMM_WORLD );
            MPI_Send ( &mat_c[slice*i], slice, MPI_DOUBLE, i, 200+i, MPI_COMM_WORLD );
        }
    }
    else
    {
        mat_a = (double*)malloc(slice*sizeof(double));
        mat_b = (double*)malloc(size*size*sizeof(double));
        mat_c = (double*)malloc(slice*sizeof(double));

        printf("%d: Receiving matrices...\n", mr);

        MPI_Recv ( &mat_a[0], slice, MPI_DOUBLE, 0, mr, MPI_COMM_WORLD, &st );
        MPI_Recv ( &mat_b[0], size*size, MPI_DOUBLE, 0, 100+mr, MPI_COMM_WORLD, &st );
        MPI_Recv ( &mat_c[0], slice, MPI_DOUBLE, 0, 200+mr, MPI_COMM_WORLD, &st );
    }

    printf("%d: Processing...\n", mr);

    mmult(size, mr, mat_a, mat_b, mat_c);

    if(mr == 0)
    {
        printf("%d: Receiving result matrix...\n", mr);

        for(int i=1; i<nproc; i++)
        {
            MPI_Recv ( &mat_c[slice*i], slice, MPI_DOUBLE, i, 500+i, MPI_COMM_WORLD, &st );
        }
    }
    else
    {
        printf("%d: Sending result matrix...\n", mr);

        MPI_Send ( &mat_c[0], slice, MPI_DOUBLE, 0, 500+mr, MPI_COMM_WORLD );
    }

    if(mr == 0)
    {
        printf("%d: Writing results...\n", mr);
        mwrite(size, mat_c, filename );
        printf("%d: Done.\n", mr);
    }

    free(mat_a);
    free(mat_b);
    free(mat_c);

    MPI_Finalize();

    return 0;
}
