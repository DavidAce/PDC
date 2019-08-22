//
// Created by david on 2019-08-22.
//

#include <stats/stats.h>
#include <vector>
#include <numeric>
#include <cmath>

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