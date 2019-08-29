//
// Created by david on 2019-08-28.
//

//
// Created by david on 2019-08-28.
//


#include <string>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
constexpr int STL_HDR_SIZE = 80;

typedef struct {
    float n[3];			/* Normal vector */
    float v1[3];			/* Vertex 1 */
    float v2[3];			/* Vertex 2 */
    float v3[3];			/* Vertex 3 */
    uint16_t attrib;		/* Attribute byte count */
} __attribute__((packed))
        stl_triangle_cpp;
MPI_Datatype stl_triangle_mpi_struct;
MPI_Datatype stl_triangle_mpi_packed;


typedef struct {
    char hdr[STL_HDR_SIZE];	/* Header */
    uint32_t n_tri;		/* Number of triangles */
    std::vector<stl_triangle_cpp> tri;
} stl_model_cpp;



void stl_read(const std::string &fname, stl_model_cpp &model) {
    int pe_size, pe_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &pe_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &pe_rank);

    MPI_File infile;
    int err = MPI_File_open(MPI_COMM_WORLD,fname.c_str(),MPI_MODE_RDONLY, MPI_INFO_NULL,&infile );
    if (err < 0) throw std::runtime_error("File not found");
    /* Read STL header */
    MPI_File_read_all(infile, model.hdr, STL_HDR_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);

    if (pe_rank == 0) std::cout << "Reading STL file: " << fname << std::endl;
    /* Make sure it's a binary STL file */
    if (strncmp(model.hdr, "solid", 5) == 0) {
        fprintf(stderr, "ASCII STL files not supported!\n");
        exit(-1);
    }
    uint32_t n_tri;
    MPI_File_read_all(infile, &n_tri, 1, MPI_UINT32_T, MPI_STATUS_IGNORE);
    model.n_tri = (n_tri + pe_size - pe_rank - 1) / pe_size;
    printf("ID %d: Found: %d triangles\n", pe_rank, model.n_tri);

    int size_struct;
    MPI_Offset filesize,position;
    MPI_File_get_size(infile,&filesize);
    MPI_File_get_position(infile, &position);
    MPI_Type_size(stl_triangle_mpi_packed,&size_struct);
    /* Allocate memory for triangles, and read them */
    model.tri.resize(model.n_tri);
//    model.tri = new stl_triangle_cpp[model.n_tri];
    /* Define offsets */
    MPI_File_seek( infile, 0, MPI_SEEK_SET );
    uint offset = 0;
    MPI_Exscan(&model.n_tri, &offset,  1, MPI_UNSIGNED, MPI_SUM, MPI_COMM_WORLD);

    MPI_Offset byte_offset = STL_HDR_SIZE * sizeof(char) + sizeof(uint32_t) + offset * size_struct;
    std::cout   << "Read ID: "      << pe_rank
                << " file size: "   << filesize
                << " position: "    << position
                << " n_tri: "       << n_tri
                << " model.n_tri: " << model.n_tri
                << " read bytes: "  << model.n_tri * size_struct
                << " allocated: "   << sizeof(model.tri[0]) * model.n_tri
                << " offset: "      << offset
                << " byte_offset: " << byte_offset
                << std::endl;


    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Status status;
    int err_read = MPI_File_read_at_all(infile, byte_offset , model.tri.data(), model.n_tri, stl_triangle_mpi_packed, &status );
//    MPI_Barrier(MPI_COMM_WORLD);
//    if (err_read != 0) throw std::runtime_error("ID " + std::to_string(pe_rank) + "could not read");
//    if (status.MPI_ERROR <= 0){
//        std::cout << "ID " << pe_rank <<  " could not read, status error: " << status.MPI_ERROR << std::endl;
//        exit(1);
//        throw std::runtime_error("ID " + std::to_string(pe_rank) + " could not read, status error: " + std::to_string(status.MPI_ERROR));
//        MPI_Finalize();
//    }
//    MPI_Barrier(MPI_COMM_WORLD);
    MPI_File_close(&infile);
    if (pe_rank == 0) printf("Done\n");

}

void stl_write(const std::string &fname, stl_model_cpp &model) {
    int pe_size, pe_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &pe_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &pe_rank);
    MPI_File outfile;
    int err = MPI_File_open(MPI_COMM_WORLD,fname.c_str(),MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL,&outfile );
    if (err < 0) throw std::runtime_error("File not found");
    if (pe_rank == 0) std::cout << "Writing STL file: " << fname << std::endl;

    /* Write STL header */
    MPI_File_write_all(outfile, model.hdr, STL_HDR_SIZE, MPI_CHAR,MPI_STATUS_IGNORE);

    /* Write number of triangles */
    uint ntotal;
    MPI_Allreduce(&model.n_tri, &ntotal, 1,MPI_UNSIGNED,MPI_SUM,MPI_COMM_WORLD);
    MPI_File_write_all(outfile, &ntotal,1,MPI_UNSIGNED, MPI_STATUS_IGNORE);
    if (pe_rank == 0) printf("Writing: %d triangles\n", ntotal);



    int size_struct;
    MPI_Offset filesize,position;
    MPI_File_get_size(outfile,&filesize);
    MPI_File_get_position(outfile, &position);
    MPI_Type_size(stl_triangle_mpi_packed,&size_struct);
    /* Write all triangles */
    uint offset = 0;
    MPI_Exscan(&model.n_tri, &offset, 1,MPI_UNSIGNED, MPI_SUM, MPI_COMM_WORLD);

    MPI_Type_size(stl_triangle_mpi_packed,&size_struct);
    MPI_Offset byte_offset = STL_HDR_SIZE * sizeof(char) + sizeof(uint32_t) + offset * sizeof(stl_triangle_cpp);

    std::cout   << "Write ID: "     << pe_rank
                << " file size: "   << filesize
                << " position: "    << position
                << " ntotal: "      << ntotal
                << " model.n_tri: " << model.n_tri
                << " write bytes: " << model.n_tri * size_struct
                << " allocated: "   << sizeof(model.tri[0]) * model.n_tri
                << " offset: "      << offset
                << " byte_offset: " << byte_offset
                << std::endl;


    MPI_File_write_at_all(outfile, byte_offset, model.tri.data(), model.n_tri,stl_triangle_mpi_packed, MPI_STATUS_IGNORE);
    MPI_File_close(&outfile);
    if (pe_rank == 0) printf("Wrote: %d triangles\n", ntotal);
    if (pe_rank == 0) printf("Done\n");
}


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    stl_model_cpp model;
    int pe_size, pe_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &pe_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &pe_rank);

    int len[5] = {3,3,3,3,1};
    MPI_Datatype types[5] = {MPI_FLOAT,MPI_FLOAT,MPI_FLOAT,MPI_FLOAT,MPI_UNSIGNED_SHORT};


    MPI_Aint displ[5] = {
            offsetof(stl_triangle_cpp, n),
            offsetof(stl_triangle_cpp, v1),
            offsetof(stl_triangle_cpp, v2),
            offsetof(stl_triangle_cpp, v3),
            offsetof(stl_triangle_cpp, attrib)};

    int err;

    err = MPI_Type_create_struct(5,len, displ,types,&stl_triangle_mpi_struct);
    if (err != 0 ) throw std::runtime_error("Error creating struct: stl_triangle_mpi_struct");
    err = MPI_Type_commit(&stl_triangle_mpi_struct);
    if (err != 0 ) throw std::runtime_error("Error commiting struct: stl_triangle_mpi_struct");

    MPI_Type_dup(stl_triangle_mpi_struct,&stl_triangle_mpi_packed);
    err = MPI_Type_commit(&stl_triangle_mpi_packed);

    if (err != 0 ) throw std::runtime_error("Error commiting struct: stl_triangle_mpi_packed");

    if(pe_rank == 0){
        int size_packed,size_struct;
        MPI_Type_size(stl_triangle_mpi_struct,&size_struct);
        MPI_Type_size(stl_triangle_mpi_packed,&size_packed);

        std::cout << "Size of stl_triangle_cpp        " << sizeof(stl_triangle_cpp) << std::endl;
        std::cout << "Size of stl_triangle_mpi_packed " << size_packed << std::endl;
        std::cout << "Size of stl_triangle_mpi_struct " << size_struct << std::endl;

    }


    stl_read("data/sphere.stl", model);
    stl_write("data/out.stl", model);

    MPI_Finalize();

    return 0;
}


