#include <iostream>
#include <mpi.h>

//Sumar datos
int sumar_datos(int* data, int size){
    int suma = 0;
    for (int i = 0; i<size; i++) {
        suma = suma + data[i];
    }
    return suma;
}

//****************************** MAIN ******************************
int main(int argc, char** argv){

    //1. Inicializar MPI
    MPI_Init(&argc, &argv);

    //2. Obtener el RANK y el # de procesos
    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//rank
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);//nproc

    if(rank==0){

        //Solamente el rank 0 tiene los datos completos
        int data[100];
        int bloque = 100/nprocs;
        std::printf("[RANK-0] Bloque: %d\n", bloque);

        std::printf("[RANK-0] MPI group size: %d\n", nprocs);

        //Incializar datos (siempre se inicializa en el Rank 0 y antes del envio)
        for (int i = 0 ; i < 100 ; i++) {
            data[i] = i;
        }

        /**
         * [x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x]
         * 0        25        50        75
         * r0 : 0   = bloque * nprocs - 1
         * r1 : 25  = bloque * rank - 1
         * r2 : 50  = bloque * rank - 1
         * r3 : 75  = bloque * rank - 1
         */
        //Para enviar a todos los procesos(#cores)
        for(int nRank=1;nRank<nprocs; nRank++){
            //int sumaTmp;
            int* start = &data[bloque*(nRank-1)];
            MPI_Send(start, bloque,MPI_INT,nRank, 0, MPI_COMM_WORLD);
        }


//        int suma0 = sumar_datos(&data[bloque*(nprocs-1)],100-(nprocs-1)*bloque);
        int suma0 = sumar_datos(&data[bloque*(nprocs-1)],bloque);
        int sumaTotal = suma0;

        //std::printf("[RANK-0] sumaTotal: %d\n", sumaTotal);
               //-- 999. Recibiendo la suma
               for(int nRank=1;nRank<nprocs; nRank++){
                   int sumaTmp;
                   MPI_Recv(&sumaTmp, 1, MPI_INT, nRank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                   std::printf("[RANK-0] Resultado rank-%d: %d\n",nRank, sumaTmp);
                   sumaTotal = sumaTotal + sumaTmp;
               }
               std::printf("[RANK-0] Suma: %d\n",sumaTotal);

    }else{

        int bloque = 100/nprocs;
        int* dataTmp = new int[bloque];//Solo toca enviar el numero de elmentos por la particion

        std::printf("Soy el rank %d recibiendo datos\n", rank);
        MPI_Recv(dataTmp, bloque, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //Concaternar en un String para imprimir los datos completos
        std::string str = "";
        for (int i = 0; i < bloque; i++) {
            str = str + std::to_string(dataTmp[i]) + ", ";
        }
        std::printf("[RANK-%d] %d, datos recibidos: [%s]\n", rank, bloque, str.c_str());

        //-- 999. Sumar los datos y enviar la suma al Rank 0
        int suma = sumar_datos(dataTmp,bloque);
        MPI_Send(&suma, 1,MPI_INT, 0, 0, MPI_COMM_WORLD);

        delete[] dataTmp;
    }

    //3. Finalizar
    MPI_Finalize();
    return 0;
}