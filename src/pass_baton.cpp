//
// Created by david on 2019-08-27.
//

#include <mpi.h>

int main (int argc, char *argv[])
{
    int work_id, work_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &work_id);
    MPI_Comm_size(MPI_COMM_WORLD, &work_size);
    //Let only ID == 0 know what the secret is
    int secret;
    if (work_id == 0){
        secret = 41;
        printf("ID %d sending  secret: %d\n", work_id, secret);
    }

    for (int send_id = 0; send_id < work_size - 1; send_id++){
        if (work_id == send_id){
            MPI_Send(&secret,1,MPI_INT,send_id+1, work_id,MPI_COMM_WORLD);
        }
        else if (work_id == send_id+1){
            MPI_Recv(&secret,1,MPI_INT,send_id, send_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("ID %d received secret: %d\n", work_id,secret);
            secret = secret + 1;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}