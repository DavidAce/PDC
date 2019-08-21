 

#include <omp.h>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::pair<double,double> get_stats(std::vector<double> & v){

    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    double mean = sum / v.size();

    double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
    double stdev = std::sqrt(std::abs(sq_sum / v.size() - mean * mean));
//    std::cout << "sq_sum: " << sq_sum << std::endl;
//    std::cout << "v.size: " << v.size() << std::endl;
//    std::cout << "mean:   " << mean << std::endl;
//    std::cout << "stdev:  " << stdev << std::endl;
    return std::make_pair(mean, stdev);
}

void compute_pi(int num, int reps){
    auto log = spdlog::get("OMP");

    std::vector<double> pi;
    std::vector<double> time;
    for (int r = 0; r < reps; r++){
        double t  = omp_get_wtime();
        constexpr long num_steps = 100000000;
        double sum = 0.0;
        double step = 1.0/num_steps;
        #pragma omp parallel for reduction(+:sum) shared(step) default(none) schedule(static) num_threads(num)
        for (long i = 0; i < num_steps; i++){
            double x = (i+0.5)*step;
            sum += 4.0/(1.0+x*x);
        }
        pi.emplace_back(sum*step);
        time.emplace_back(1000 *(omp_get_wtime()-t));
    }
    auto [pi_mean,pi_std] = get_stats(pi);
    auto [time_mean,time_std] = get_stats(time);
    log->info("pi = {:.16f} +- {:20.16f}. Time: {:.5f} +- {:.5f} ms. Threads: {}", pi_mean, pi_std, time_mean,time_std, num);

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


    compute_pi(1,10);
    compute_pi(2,10);
    compute_pi(4,10);
    compute_pi(8,10);
    compute_pi(16,10);
    compute_pi(32,10);
    compute_pi(64,10);
    compute_pi(128,10);
    return 0;

}