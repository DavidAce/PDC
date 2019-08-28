#include <stdio.h>
#include <mpi.h>

int main (int argc, char *argv[])
{
  int myrank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  MPI_File hellofile;
  MPI_File_open(MPI_COMM_WORLD,"data/helloworld.txt", MPI_MODE_WRONLY, MPI_INFO_NULL,&hellofile );

  std::string hello = "Processor " + std::to_string(myrank) + " of " + std::to_string(size) + ": Hello World!\n";

  int offset = myrank * hello.size();
  int count = hello.size();
  MPI_File_write_at_all(hellofile, offset, hello.c_str(), count, MPI_CHAR, MPI_STATUS_IGNORE);
  MPI_File_close(&hellofile);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}
