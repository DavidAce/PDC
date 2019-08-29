//
// Created by david on 2019-08-28.
//


#include <string>
#include <mpi.h>
#include <fstream>
#include <iostream>
#include <vector>

constexpr int STL_HDR_SIZE = 80;

typedef struct {
    float n[3];			/* Normal vector */
    float v1[3];			/* Vertex 1 */
    float v2[3];			/* Vertex 2 */
    float v3[3];			/* Vertex 3 */
    uint16_t attrib;		/* Attribute byte count */
} __attribute__((packed))
        stl_triangle_cpp;


typedef struct {
    char hdr[STL_HDR_SIZE];	/* Header */
    uint32_t n_tri;		/* Number of triangles */
    std::vector<stl_triangle_cpp> tri;
} stl_model_cpp;



void stl_read(const std::string &fname, stl_model_cpp &model) {
    int pe_size, pe_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &pe_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &pe_rank);


    std::ifstream infile (fname, std::ifstream::binary);
    if (not infile.good()) throw std::runtime_error("File not found");
    /* Read STL header */
    infile.read(model.hdr,STL_HDR_SIZE*sizeof(char));

    if (pe_rank == 0) std::cout << "Reading STL file: " << fname << std::endl;
    /* Make sure it's a binary STL file */
    if (strncmp(model.hdr, "solid", 5) == 0) {
        fprintf(stderr, "ASCII STL files not supported!\n");
        exit(-1);
    }

    /* Read how many triangles the file contains */
    infile.read((char*)&model.n_tri,1*sizeof(uint32_t));
    if (pe_rank == 0) printf("Found: %d triangles\n", model.n_tri);

    /* Allocate memory for triangles, and read them */
    model.tri.resize(model.n_tri);
    infile.read((char*)model.tri.data(), model.n_tri * sizeof(stl_triangle_cpp) );
    infile.close();
    if (pe_rank == 0) printf("Done\n");

}

void stl_write(const std::string &fname, stl_model_cpp &model) {
    int pe_size, pe_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &pe_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &pe_rank);
    std::ofstream outfile (fname, std::ifstream::binary);
    if (pe_rank == 0) std::cout << "Writing STL file: " << fname << std::endl;

    /* Write STL header */
    outfile.write(model.hdr,STL_HDR_SIZE*sizeof(char));


    /* Write number of triangles */
    outfile.write((char*)&model.n_tri,1*sizeof(uint32_t));
    if (pe_rank == 0) printf("Wrote: %d triangles\n", model.n_tri);



    /* Write all triangles */
    outfile.write((char*)model.tri.data(), model.n_tri * sizeof(stl_triangle_cpp) );
    outfile.close();
    if (pe_rank == 0) printf("Done\n");
}


int main(int argc, char **argv) {
    stl_model_cpp model;

    MPI_Init(&argc, &argv);

    stl_read("data/sphere.stl", model);
    stl_write("data/out.stl", model);

    MPI_Finalize();

    return 0;
}


