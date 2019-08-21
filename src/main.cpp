 

#include <omp.h>
#include <iostream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void compute_pi(int num){
    auto log = spdlog::get("OMP");
    constexpr long num_steps = 1000000000;
    double time  = omp_get_wtime();
    double sum = 0.0;
    double step = 1.0/num_steps;


    #pragma omp parallel for reduction(+:sum) shared(step) default(none) schedule(static) num_threads(num)
    for (long i = 0; i < num_steps; i++){
        double x = (i+0.5)*step;
        sum += 4.0/(1.0+x*x);
    }
    log->info("pi = {:.16f}. Time: {:.4f} ms. Threads: {}", sum*step, 1000 *(omp_get_wtime()-time), num);

}


int main (){
    auto log = spdlog::stdout_color_mt("OMP");
    log->set_pattern("[%Y-%m-%d %H:%M:%S][%n]%^[%=8l]%$ %v");

    omp_set_num_threads(32);
    #pragma omp parallel for ordered shared(log) default(none)
    for (int i = 0; i < omp_get_num_threads(); i++){
        #pragma omp ordered
        log->info("hello from id {}", omp_get_thread_num());
    }


    compute_pi(1);
    compute_pi(2);
    compute_pi(4);
    compute_pi(8);
    compute_pi(16);
    compute_pi(32);
    compute_pi(64);
    compute_pi(128);
    return 0;

}