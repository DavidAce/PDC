/*
 *  shwater2d.c solves the two dimensional shallow water equations
 *  using the Lax-Friedrich's scheme
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <omp.h>
#include <sys/time.h>

#include <shwater2d/shwater2d_modern.h>
#include <iostream>
#include <math/nmspc_tensor_extra.h>


/* Timing function */
double Shwater2D::gettime() {
    struct timeval tv;
    gettimeofday(&tv,nullptr);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

/* Check that the solution is finite */
void Shwater2D::validate(const TType &Q) {
    auto Qmap = Eigen::Map<const VType>(Q.data(),Q.size());
    if (not Qmap.allFinite()){
        throw std::runtime_error("Invalid solution\n");
    }
}


/* Flux function in the x-direction */
void Shwater2D::fx(TType &Q) {
    int i,j;
    #pragma omp parallel for default(shared) private(i,j) collapse(2)
    for (j = 0; j < n; j++){
        for (i = 0; i < m; i++) {
            fq(i,j,0) = Q(i, j, 1);
            fq(i,j,1) = std::pow(Q(i, j, 1), 2) / Q(i, j,0)
                      + std::pow(Q(i, j, 0), 2) / 2.0 * g;
            fq(i,j,2) = Q(i, j, 1) * Q(i, j,2) / Q(i, j,0);
        }
    }
}

void Shwater2D::Fx(TType &Q, double dxdt){
    int i,j,k;
    #pragma omp parallel for default(shared) private(i,j,k) collapse(3)
    for (k = 0; k < cell_size; k++){
        for(j = 0; j < n; j++ ){
            for (i = 1; i < m; i++){
                nFq(i,j,k) = 0.5 * ((fq(i-1,j,k) + fq(i,j,k)) -
                                  dxdt * (Q(i,j,k) - Q(i-1,j,k)));
            }
        }

    }
}


/* Flux function in the y-direction */
void Shwater2D::fy(TType &Q) {
    int i,j;
    #pragma omp parallel for default(shared) private(i,j) collapse(2)
    for (j = 0; j < n; j++) {
        for (i = 0; i < m; i++){
            fq(i,j,0) = Q(i, j, 2);
            fq(i,j,1) = Q(i, j, 1) * Q(i, j,2) / Q(i, j, 0);
            fq(i,j,2) = std::pow(Q(i, j, 2), 2) / Q(i, j, 0)
                      + std::pow(Q(i, j, 0), 2) / 2.0 * g;
        }
    }
}

void Shwater2D::Fy(TType &Q, double dydt){
    int i,j,k;
    #pragma omp parallel for default(shared) private(i,j,k) collapse(3)
    for (k = 0; k < cell_size; k++){
        for(j = 1; j < n; j++ ){
            for (i = 0; i < m; i++){
                nFq(i,j,k) = 0.5 * ((fq(i,j-1,k) + fq(i,j,k)) -
                                    dydt * (Q(i,j,k) - Q(i,j-1,k)));
            }
        }
    }
}

/*
  This is the Lax-Friedrich's scheme for updating volumes
  Try to parallelize it in an efficient way!
*/
void Shwater2D::laxf_scheme_2d(TType &Q, double dx, double dy, double dt) {
    double dtdy = dt/dy;
    double dtdx = dt/dx;
    double dydt = dy/dt;
    double dxdt = dx/dt;
    fx(Q);
    Fx(Q,dxdt);
    t_loop1.tic();


    /* Calculate and update fluxes in the x-direction */
    int i,j,k;
    #pragma omp parallel for default(shared) private(i,j) collapse(3)
    for (k = 0; k < cell_size;  k++) {
        for(j = 0; j < n; j++) {
            for (i = 1; i < m-1; i++){
                Q(i,j,k) -= dtdx * ((nFq(i+1,j,k) - nFq(i,j,k)));
            }
        }
    }


    t_loop1.toc();
    fy(Q);
    Fy(Q,dydt);
    t_loop2.tic();
    {
        /* Calculate and update fluxes in the y-direction */
        int i,j,k;
        #pragma omp parallel for default(shared) private(i,j,k) collapse(3)
        for (k = 0; k < cell_size; k++) {
            for (j = 1; j < n-1; j++){
                for (i = 0; i < m; i++) {
                    Q(i, j, k) -= dtdy * ((nFq(i,j+1, k) - nFq(i,j, k)));
                }
            }
        }
    }
    t_loop2.toc();
}

/*
  This is the main solver routine, parallelize this.
  But don't forget the subroutine laxf_scheme_2d
*/
void Shwater2D::solver(TType &Q,double dx, double dy, double dt) {
    Eigen::Array3d bc_mask = {1.0, -1.0, -1.0};
    int steps = ceil(tend / dt);
    for (int t = 0; t < steps; t++) {

        /* Apply boundary condition */
        for (int k = 0; k < cell_size; k++) {
            for (int j = 1; j < n - 1 ; j++) {
                Q(0  , j, k) = bc_mask[k] *  Q(1  , j,k);
                Q(m-1, j, k) = bc_mask[k] *  Q(m-2, j,k);
            }
            for (int i = 0; i < m; i++)  {
                Q(i, 0  , k) = bc_mask[k] * Q(i, 1  ,k);
                Q(i, n-1, k) = bc_mask[k] * Q(i, n-2,k);
            }
        }
        /* Update all volumes with the Lax-Friedrich's scheme */
        laxf_scheme_2d(Q, dx, dy, dt);
    }
    t_loop1.print_total_reset();
    t_loop2.print_total_reset();
}

/*
  This is the main routine of the program, which allocates memory
  and setup all parameters for the problem.

  You don't need to parallelize anything here!

  However, it might be useful to change the m and n parameters
  during debugging
*/
int Shwater2D::shwater2d_run() {

    /* Initialize timers */
    t_loop1.set_properties(true, 5, "Loop1: ");
    t_loop2.set_properties(true, 5, "Loop2: ");

    /* Allocate memory for the domain */
    TType Q(m,n,cell_size);
    fq = TType(m,n,cell_size);
    nFq= TType(m,n,cell_size);
    VType x = VType::LinSpaced(m,xstart, xend);
    VType y = VType::LinSpaced(n,ystart, yend);
    double dx = x[1] - x[0];
    double dy = y[1] - y[0];
    double dt = dx / sqrt( g * 5.0);


    /* Set initial Gauss hump */
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            Q(i, j, 0) = 4.0;
            Q(i, j, 1) = 0.0;
            Q(i, j, 2) = 0.0;
        }
    }

    for (int i = 1; i < m-1; i++) {
        for (int j = 1; j < n-1; j++) {
            Q(i, j, 0) = 4.0 + epsi * exp(-(std::pow(x[i] - xend / 4.0, 2) + std::pow(y[j] - yend / 4.0, 2)) /
                                          (std::pow(delta, 2)));
        }
    }

    double stime = gettime();
    solver(Q, dx, dy, dt);
    double etime = gettime();

    validate(Q);

    printf("C++ Solver took %g seconds\n", etime - stime);

    /* Uncomment this line if you want visualize the result in ParaView */
    /* save_vtk(Q, x, y, m, n); */


    return 0;
}


