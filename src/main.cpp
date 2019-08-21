 

#include <omp.h>
#include <iostream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
int main (){
    auto log = spdlog::stdout_color_mt("OMP");
    log->set_pattern("[%Y-%m-%d %H:%M:%S][%n]%^[%=8l]%$ %v");

    omp_set_num_threads(8);
    #pragma omp parallel for ordered
    for (int i = 0; i < omp_get_num_threads(); i++){
        #pragma omp ordered
        log->info("hello from id {}", omp_get_thread_num());
    }



    constexpr long num_steps = 10000000;
    double time  = omp_get_wtime();
    double sum = 0.0;
    double step = 1.0/num_steps;

    #pragma omp parallel for reduction(+:sum) shared(step) default(none) schedule(static)
    for (long i = 0; i < num_steps; i++){
        double x = (i+0.5)*step;
        sum += 4.0/(1.0+x*x);
    }
    log->info("pi = {:.16f}. Time: {:.4f} ms", sum*step, 1000 *(omp_get_wtime()-time));
    return 0;

}