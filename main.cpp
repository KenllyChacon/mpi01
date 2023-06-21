#include <iostream>
#include <mpi.h>

int main(int argc, char** argv){

    //1. Inicializar MPI
    MPI_Init(&argc, &argv);

    //2. Obtener el RANK y el # de procesos
    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//rank
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);//rank

    if(rank==0){
        std::printf("MPI group size: %d\n", nprocs);
    }else{
        std::printf("Soy el rank %d\n", rank);
    }

    //std::printf("Soy el rank %d de un total de %d procesos!\n", rank, nprocs);    

    //while(1);

    //3. Finalizar
    MPI_Finalize();
    return 0;
}