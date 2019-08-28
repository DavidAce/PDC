//
// Created by david on 2019-08-27.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <cmath>
#include <numeric>


std::pair<double,std::vector<int>> load_file(const std::string filename){
    int target;
    std::vector<int> b;

    /* File b.data has the target value on the first line
       The remaining 300 lines of b.data have the values for the b array */
    std::ifstream infile (filename);
    if (not infile.good()) throw std::runtime_error("File not found");
    /* read in target */

    std::string line;
    bool first = true;
    // Read the next line from File untill it reaches the end.
    while (std::getline(infile, line) and infile.good()){
        if (first){
            target = (int) std::strtol(line.c_str(),nullptr,10);
            first = false;
            continue;
        }
        // Line contains non-empty string then save it in vector
        if(not line.empty())
            b.push_back(std::strtol(line.c_str(),nullptr,10));
    }

    return std::make_pair(target,b);
}

std::vector<int> find_target(std::vector<int> &v, int displacement,int target ){
    std::vector<int> match_idx;
    int idx;  //Index on the global array
    for(size_t i = 0; i < v.size(); i++){
        if(v[i] == target) {
            idx =  i + displacement;
            match_idx.emplace_back(idx);
        }
    }
    return match_idx;
}


int main (int argc, char *argv[])
{

    constexpr int MASTER = 0;
    int work_id, work_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &work_id);
    MPI_Comm_size(MPI_COMM_WORLD, &work_size);

    std::vector<int> global_array;
    std::vector<int> local_array;
    std::vector<int> displacements(work_size);
    std::vector<int> sendcounts;
    int recvcount;

    int target;
    std::string outfilename = "data/found_" + std::to_string(work_id) +".data";


    if(work_id == MASTER){
        std::cout << "Loading file from master" << std::endl;
        std::tie(target,global_array) = load_file("data/b.data");
        // Split the global array into local arrays. Because the number of items may not be divisible
        // by the number of workers, we need to do this with variable lengt scatter, Scatterv.
        int global_size = global_array.size();
        int local_size  = global_size/work_size;
        int chunk_size  = local_size;
        int elems_left  = global_size;
        int offset      = 0;
        for(int id = 0; id < work_size; id++){
            if (elems_left <= 0) throw std::logic_error("Elems left < 0!");
            chunk_size = elems_left / (work_size - id);
            displacements[id] = offset;
            sendcounts   .emplace_back(chunk_size);
            offset     += chunk_size;
            elems_left -= chunk_size;
        }
    }
    MPI_Bcast(&target,1,MPI_INT,MASTER,MPI_COMM_WORLD);
    MPI_Scatter(sendcounts.data(),1,MPI_INT,&recvcount,1,MPI_INT,MASTER,MPI_COMM_WORLD);

    local_array.resize(recvcount);
    MPI_Scatterv(global_array.data(),sendcounts.data(),displacements.data(),MPI_INT,local_array.data(),recvcount,MPI_INT,MASTER,MPI_COMM_WORLD);
    MPI_Bcast(displacements.data(),work_size,MPI_INT,MASTER,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    // Let each thread search in its own local array
    auto match_array = find_target(local_array,displacements[work_id]+1,target); //Add 1 to displacement because first element in file is the target
    //Write the results to their respective files if they found anything
    if(not match_array.empty()){
        std::ofstream outfile(outfilename);
        for(auto & match :match_array){
            std::cout << "ID: " << work_id << " found idx " << match << std::endl;
            outfile << match << std::endl ;
        }
    }

    //Collect the results into master. Remember that the match_array is a variable length array.
    int matchsize = match_array.size();
    std::vector<int> matchsizes(work_size);
    std::vector<int> allmatches;
    MPI_Gather(&matchsize,1,MPI_INT,matchsizes.data(),1,MPI_INT,MASTER,MPI_COMM_WORLD);
    if(work_id == MASTER){
        int sum_matches = std::accumulate(matchsizes.begin(),matchsizes.end(),0);
        allmatches.resize(sum_matches);
        displacements = {};
        int offset = 0;
        for(auto &s : matchsizes) {
            displacements.emplace_back(offset);
            offset += s;
        }

    }
    MPI_Gatherv(match_array.data(),match_array.size(),MPI_INT,allmatches.data(),matchsizes.data(),displacements.data(),MPI_INT,MASTER,MPI_COMM_WORLD );
    //let master write all the results into a single file
    if(work_id == MASTER){
        std::ofstream masteroutfile("data/found_parallel.data");
        for(auto & match :allmatches){
            masteroutfile << match << std::endl ;
        }
    }


    MPI_Finalize();


    return 0;
}
