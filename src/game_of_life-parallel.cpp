/***********************

Conway Game of Life

Parallel version

************************/



#include <iostream>
#include <Eigen/Core>
#include <random>
#include <mpi.h>
#include "class_tic_toc.h"
std::mt19937 rng;
double uniform_int(){
    std::uniform_int_distribution<>  rand_real(0,1);
    return rand_real(rng);
}


template<typename T1, typename T2>
inline auto mod(const T1 x, const T2 y){
    return (x % y + y) % y;
}




int main(int argc, char *argv[])
{
    int work_id, work_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &work_id);
    MPI_Comm_size(MPI_COMM_WORLD, &work_size);

    class_tic_toc t_pi;
    constexpr int ghost_cols = 2;
    constexpr int NI     = 200;        /* array sizes */
    constexpr int NJ     = 200 + ghost_cols;    /* Width of each board */
    constexpr int NSTEPS = 500;    /* number of time steps */

    int nsum, isum;
    /* initialize elements of old to 0 or 1 */
    rng.seed(work_id);
    auto uni = [&](){ return uniform_int(); };
    Eigen::ArrayXXi old_board = Eigen::ArrayXXi::NullaryExpr(NI,NJ,uni);
    Eigen::ArrayXXi new_board(NI,NJ);
    Eigen::ArrayXi left_col_send(NI);
    Eigen::ArrayXi rite_col_send(NI);
    Eigen::ArrayXi left_col_recv(NI);
    Eigen::ArrayXi rite_col_recv(NI);

    /*  time steps */
    for(int n=0; n<NSTEPS; n++){
        for(int j = 1; j < NJ-1; j++){
            for (int i = 0; i < NI; i++){
                int im = mod(i-1,NI);
                int ip = mod(i+1,NI);
                int jm = mod(j-1,NJ);
                int jp = mod(j+1,NJ);
                nsum =      old_board(im,jp) + old_board(i,jp) + old_board(ip,jp)
                          + old_board(im,j )                   + old_board(ip,j )
                          + old_board(im,jm) + old_board(i,jm) + old_board(ip,jm);
                switch(nsum){

                    case 3:
                        new_board(i,j) = 1;
                        break;
                    case 2:
                        new_board(i,j) = old_board(i,j);
                        break;
                    default:
                        new_board(i,j) = 0;
                }
            }
        }


        /* Syncronize borders */
        int nb_left = mod(work_id-1,work_size);
        int nb_rite = mod(work_id+1,work_size);

        left_col_send = new_board.col(1);
        rite_col_send = new_board.col(NJ-2);
        left_col_recv(NI);
        rite_col_recv(NI);
        MPI_Sendrecv(rite_col_send.data(),NI,MPI_INT,nb_rite,work_id,left_col_recv.data(),NI,MPI_INT,nb_left,nb_left,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Sendrecv(left_col_send.data(),NI,MPI_INT,nb_left,work_id,rite_col_recv.data(),NI,MPI_INT,nb_rite,nb_rite,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        new_board.col(0)    = left_col_recv;
        new_board.col(NJ-1) = rite_col_recv;

        /* copy new state into old state */
        old_board = new_board;

    }



        /* There are two borders per board, left and right columns.
         * There left column of the leftmost board is equal

        MPI_


    }

    /*  Iterations are done; sum the number of live cells */
    isum = old_board.sum();
    printf("\nNumber of live cells = %d\n", isum);
    MPI_Finalize();

    return 0;
}
