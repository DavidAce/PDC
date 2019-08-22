//
// Created by david on 2019-08-22.
//

#ifndef OMPADVANCED_SHWATER2D_MODERN_H
#define OMPADVANCED_SHWATER2D_MODERN_H
#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <general/class_tic_toc.h>
class Shwater2D{
private:


    using MType = Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::ColMajor>;
    using VType = Eigen::Matrix<double,Eigen::Dynamic,1,Eigen::ColMajor>;
    using TType  = Eigen::Tensor<double,3, Eigen::ColMajor>;
    using TTypeR = Eigen::Tensor<double,3, Eigen::RowMajor>;
//    using TType2 = Eigen::Tensor<double,2, Eigen::ColMajor>;
    static constexpr int    cell_size = 3;
    static constexpr double xstart    = 0.0;
    static constexpr double ystart    = 0.0;
    static constexpr double xend      = 4.0;
    static constexpr double yend      = 4.0;

    /*
   epsi      Parameter used for initial condition
   delta     Parameter used for initial condition
   dx        Distance between two volumes (x-direction)
   dy        Distance between two volumes (y-direction)
   dt        Time step
   tend      End time
   */

    static constexpr double g         = 9.81;
    static constexpr double epsi      = 2.0;
    static constexpr double delta     = 0.5;
    static constexpr double tend      = 0.1;
    /* Use m volumes in the x-direction and n volumes in the y-direction */
    static constexpr int    m         = 1000;
    static constexpr int    n         = 1000;
    TType fq ;
    TType nFq;

    double gettime();
    void validate(const TType &Q);
    void  fx(TType &Q);
    void  Fx(TType &Q,  double dxdt);

    void fy(TType &Q);
    void Fy(TType &Q, double dydt);
    void laxf_scheme_2d(TType &Q, double dx, double dy, double dt);
    void solver(TType &Q, double dx, double dy, double dt);

public:
    class_tic_toc t_loop1;
    class_tic_toc t_loop2;
    Shwater2D() = default;
    int shwater2d_run();

};

#endif //OMPADVANCED_SHWATER2D_MODERN_H

