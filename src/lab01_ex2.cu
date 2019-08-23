
#include <stdio.h>
#include <sys/time.h>

#define BLOCK_SIZE 256
#define ARRAY_SIZE 16777216

typedef struct timeval tval;

/**
 * Helper method to generate a very naive "hash".
 */
float generate_hash(int n, float *y)
{
    float hash = 0.0f;
    
    for (int i = 0; i < n; i++)
    {
        hash += y[i];
    }
    
    return hash;
}

/**
 * Helper method that calculates the elapsed time between two time intervals (in milliseconds).
 */
long get_elapsed(tval t0, tval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_usec - t0.tv_usec) / 1000;
}

/**
 * SAXPY reference implementation using the CPU.
 */
void cpu_saxpy(int n, float a, float *x, float *y)
{
    for (int i = 0; i < n; i++)
    {
        y[i] = a * x[i] + y[i];
    }
}

////////////////
// TO-DO #2.6 /////////////////////////////////////////////////////////////
// Declare the kernel gpu_saxpy() with the same interface as cpu_saxpy() //
///////////////////////////////////////////////////////////////////////////
__global__ void gpu_saxpy(float a, float *x, float *y)
{
    const int i = blockIdx.x*blockDim.x + threadIdx.x;
    if (i > ARRAY_SIZE) y[i] = 0 ;
    else  y[i] = a * x[i] + y[i];
}



int compute_num_blocks(){
    int i = ARRAY_SIZE/BLOCK_SIZE;
    int imax = ARRAY_SIZE/BLOCK_SIZE + BLOCK_SIZE;
    while (ARRAY_SIZE % i != 0 and i <= imax ){
        i++;
    }
    return i;
}

int pad_arraysize(){
    int i = ARRAY_SIZE;
    while(i % 32 != 0){
        i++;
    }
    return i;
}


int main(int argc, char **argv)
{
    float a     = 0.0f;
    float *x    = NULL;
    float *y    = NULL;
    float error = 0.0f;

    ////////////////
    // TO-DO #2.2 ///////////////////////////////
    // Introduce the grid and block definition //
    /////////////////////////////////////////////
    const int NUM_BLOCKS = compute_num_blocks();
    dim3 grid(NUM_BLOCKS  ,1,1); // 1 block in the grid
    dim3 block(BLOCK_SIZE,1,1); // 256 threads per block

    printf("grid: %d,  block: %d\n", NUM_BLOCKS, BLOCK_SIZE);

    //////////////////
    // TO-DO #2.3.1 /////////////////////////////
    // Declare the device pointers d_x and d_y //
    /////////////////////////////////////////////
    float *d_x = NULL;
    float *d_y = NULL;
 

    // Make sure the constant is provided
    if (argc != 2)
    {
        fprintf(stderr, "Error: The constant is missing!\n");
        return -1;
    }
    
    // Retrieve the constant and allocate the arrays on the CPU
    a = atof(argv[1]);
    x = (float *)malloc(sizeof(float) * ARRAY_SIZE);
    y = (float *)malloc(sizeof(float) * ARRAY_SIZE);
    
    // Initialize them with fixed values
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        x[i] = 0.1f;
        y[i] = 0.2f;
    }
    
    //////////////////
    // TO-DO #2.3.2 ////////////////////////////////////////////////////////
    // Allocate d_x and d_y on the GPU, and copy the content from the CPU //
    ////////////////////////////////////////////////////////////////////////
    int D_ARRAY_SIZE = pad_arraysize();
    printf("Actual array size: %d\n", ARRAY_SIZE);
    printf("Padded array size: %d\n", D_ARRAY_SIZE);
    cudaMalloc(&d_x, D_ARRAY_SIZE*sizeof(float));
    cudaMalloc(&d_y, D_ARRAY_SIZE*sizeof(float));
    cudaMemcpy(d_x, x, D_ARRAY_SIZE,cudaMemcpyHostToDevice); 
    cudaMemcpy(d_y, y, D_ARRAY_SIZE,cudaMemcpyHostToDevice);
    tval starttime;
    tval endtime;
    gettimeofday(&starttime,NULL);
    // Call the CPU code
    cpu_saxpy(ARRAY_SIZE, a, x, y);
    gettimeofday(&endtime,NULL);
    int elapsed = get_elapsed(starttime,endtime);
    printf("Time elapsed %d \n", elapsed);

    // Calculate the "hash" of the result from the CPU
    error = generate_hash(ARRAY_SIZE, y);
    
    ////////////////
    // TO-DO #2.4 ////////////////////////////////////////
    // Call the GPU kernel gpu_saxpy() with d_x and d_y //
    //////////////////////////////////////////////////////
    gettimeofday(&starttime,NULL);
    gpu_saxpy<<<NUM_BLOCKS, BLOCK_SIZE>>>(a, d_x,d_y);

    //////////////////
    // TO-DO #2.5.1 ////////////////////////////////////////////////////
    // Copy the content of d_y from the GPU to the array y on the CPU //
    ////////////////////////////////////////////////////////////////////
    
    cudaMemcpy(x, d_x, ARRAY_SIZE,cudaMemcpyDeviceToHost); 
    cudaMemcpy(y, d_y, ARRAY_SIZE,cudaMemcpyDeviceToHost);
    gettimeofday(&endtime,NULL);
    elapsed = get_elapsed(starttime,endtime);
    printf("Time elapsed %d \n", elapsed);

    // Calculate the "hash" of the result from the GPU
    error = fabsf(error - generate_hash(D_ARRAY_SIZE, y));
    
    // Confirm that the execution has finished
    printf("Execution finished (error=%.6f).\n", error);
    
    if (error > 0.0001f)
    {
        fprintf(stderr, "Error: The solution is incorrect!\n");
    }
    
    // Release all the allocations
    free(x);
    free(y);
    
    //////////////////
    // TO-DO #2.5.2 /////////
    // Release d_x and d_y //
    /////////////////////////
    cudaFree(d_x); // Free the memory
    cudaFree(d_y); // Free the memory
    return 0;
}
