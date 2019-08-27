//
// Created by david on 2019-08-27.
//


#include <iostream>
#include <random>
#include <iomanip>
#include "class_tic_toc.h"
#include <mpi.h>
std::mt19937 rng;
double uniform_double(){
    std::uniform_real_distribution<>  rand_real(-1.0,1.0);
    return rand_real(rng);
}

class_tic_toc t_pi;


double dboard (int darts);
constexpr unsigned int DARTS  = 50000;     /* number of throws at dartboard */
constexpr unsigned int ROUNDS = 1000;       /* number of times "darts" is iterated */
constexpr unsigned int MASTER = 0;        /* task ID of master task */

int main (int argc, char *argv[])
{

    int work_id, work_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &work_id);
    MPI_Comm_size(MPI_COMM_WORLD, &work_size);

    t_pi.set_properties(true,5,"Time: ");
    double local_time = 0;
    double total_time = 0;


    double  homepi,         /* value of pi calculated by current task */
            pi,             /* average of pi after "darts" is thrown */
            avepi,          /* average pi value for all iterations */
            pirecv,         /* pi received from worker */
            pisum;          /* sum of workers pi values */
    int     home_throws = 0;
    int     sum_throws = 0;
    rng.seed (work_id);

    homepi = 0;
    int i  = 0;

    for (int r = work_id; r < ROUNDS; r+=work_size) {
        t_pi.tic();
        pi = dboard(DARTS);
        home_throws += DARTS;
        /* Master calculates the average value of pi over all iterations */
        homepi = ((homepi * i) + pi)/(i + 1);
        i++;
        t_pi.toc();
        local_time = t_pi.get_measured_time() * 1000; // Count in ms
        printf("ID: %d:  After %8d throws in %d rounds, the average value of pi = %10.8f. Time: %5.1f ms \n",
               work_id,home_throws,i,homepi,local_time);

    }

    // Collect worker averages in master
    MPI_Reduce(&local_time, &total_time,1,MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&home_throws, &sum_throws,1,MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&homepi, &pisum,1,MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
    if(work_id == MASTER){
        avepi = pisum / work_size;
        printf("   After %8d throws and %5.2f ms in total (%5.1f ms avg), the average value of pi = %10.8f\n",
               sum_throws,total_time, total_time/work_size,avepi);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();


    return 0;
}



/******************************************************************************
* FILE: dboard.c
* DESCRIPTION:
*   Used in pi calculation example codes.
*   See mpi_pi_send.c and mpi_pi_reduce.c
*   Throw darts at board.  Done by generating random numbers
*   between 0 and 1 and converting them to values for x and y
*   coordinates and then testing to see if they "land" in
*   the circle."  If so, score is incremented.  After throwing the
*   specified number of darts, pi is calculated.  The computed value
*   of pi is returned as the value of this function, dboard.
*   Note:  the seed value for rand() is set in pi_send.f or pi_reduce.f.
* AUTHOR: unknown
* LAST REVISED: 04/14/05 Blaise Barney
****************************************************************************/
/*
Explanation of constants and variables used in this function:
  darts       = number of throws at dartboard
  score       = number of darts that hit circle
  n           = index variable
  r           = random number between 0 and 1
  x_coord     = x coordinate, between -1 and 1
  x_sqr       = square of x coordinate
  y_coord     = y coordinate, between -1 and 1
  y_sqr       = square of y coordinate
  pi          = computed value of pi
*/




double dboard(int darts)
{
    double x_coord, y_coord, pi, r;
    int score, n;
    score = 0;

    /* "throw darts at board" */
    for (n = 1; n <= darts; n++) {
        /* generate random numbers for x and y coordinates */
        x_coord = uniform_double();
        y_coord = uniform_double();
        /* if dart lands in circle, increment score */
        if (y_coord*y_coord + x_coord*x_coord <= 1.0)
            score++;
    }

    /* calculate pi */
    pi = 4.0 * (double)score/(double)darts;
    return pi;
}
