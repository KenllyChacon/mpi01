#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>
#include <vector>
#include <algorithm>
#include <chrono>

std::vector<int> read_file() {
    std::fstream fs("/workspace/mpi01/datosUnidos.txt", std::ios::in );
    std::string line;
    std::vector<int> ret;
    while( std::getline(fs, line) ){
        ret.push_back( std::stoi(line) );
    }
    fs.close();
    return ret;
}

// Función para mezclar dos vectores ordenados en uno solo
std::vector<int> merge(const std::vector<int>& left, const std::vector<int>& right) {
    std::vector<int> merged;
    merged.reserve(left.size() + right.size());

    int i = 0; // Índice para el vector left
    int j = 0; // Índice para el vector right

    // Mezclar los elementos de los vectores left y right en orden ascendente
    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) {
            merged.push_back(left[i]);
            i++;
        } else {
            merged.push_back(right[j]);
            j++;
        }
    }

    // Agregar los elementos restantes del vector left
    while (i < left.size()) {
        merged.push_back(left[i]);
        i++;
    }

    // Agregar los elementos restantes del vector right
    while (j < right.size()) {
        merged.push_back(right[j]);
        j++;
    }

    return merged;
}


std::vector<int> copiar_parte_vector(const std::vector<int>& original, int inicio, int tamano) {
    std::vector<int> copia;
    copia.reserve(tamano); // Reservar espacio en el vector copia

    // Copiar los elementos desde el inicio hasta el inicio + tamano
    for (int i = inicio; i < inicio + tamano; i++) {
        copia.push_back(original[i]);
    }

    return copia;
}

std::vector<int> vector = read_file();

//****************************** MAIN ******************************
int main(int argc, char **argv) {

    //1. Inicializar MPI
    MPI_Init(&argc, &argv);

    //2. Obtener el RANK y el # de procesos
    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//rank
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);//nproc

    if (rank == 0) {

        std::printf("[RANK-0] Tamaño del vector: %zu\n", vector.size());

        //Solamente el rank 0 tiene los datos completos
        int bloque;
        bool es_impar = vector.size() % 2 != 0;
        if (es_impar) {
            bloque = ((vector.size()) / nprocs) + 1;
        }else{
            bloque = (vector.size()) / nprocs;
        }

        std::printf("[RANK-0] Tamaño del bloque: %d\n", bloque);

        std::printf("[RANK-0] MPI group size: %d\n", nprocs);

        /**
         * [x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x]
         * 0        25        50        75
         * r0 : 9 - 12  = bloque * (nprocs - 1)
         * r1 : 0 - 3   = bloque * (rank - 1)
         * r2 : 3 - 6   = bloque * (rank - 1)
         * r3 : 6 - 9   = bloque * (rank - 1)
         */
        //Para enviar a todos los procesos(#cores)
        for (int nRank = 1; nRank < nprocs; nRank++){
            int *start = &vector[bloque * (nRank - 1)];
            MPI_Send(start, bloque, MPI_INT, nRank, 0, MPI_COMM_WORLD);
        }

        //Sumar lo del Rank 0
        int xx = (nprocs-1)*bloque;
        std::printf("[RANK-0], XXXXXXXXXXXXXXXX %d\n",xx);

        std::vector<int> vector_cero = copiar_parte_vector(vector,(nprocs-1)*bloque,vector.size()-(nprocs-1)*bloque);

        std::string str = "";
        for (int i = 0; i < vector_cero.size(); i++) {
            str = str + std::to_string(vector_cero[i]) + ", ";
        }

        std::printf("[RANK-0], Vector Cero Original: [%s]\n", str.c_str());

        std::sort(vector_cero.begin(), vector_cero.end());

        std::string str2 = "";
        for (int i = 0; i < vector_cero.size(); i++) {
            str2 = str2 + std::to_string(vector_cero[i]) + ", ";
        }
        std::printf("[RANK-0], Vector Cero Ordenado: [%s]\n", str2.c_str());

        std::vector<int> vector_ordenado;

        //-- 999. Recibiendo la suma
        for(int nRank=1;nRank<nprocs; nRank++){
            std::vector<int> vector_parcial(bloque);
            MPI_Recv(vector_parcial.data(), bloque, MPI_INT, nRank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            std::string stra = "";
            for (int i = 0; i < vector_parcial.size(); i++) {
                stra = stra + std::to_string(vector_parcial[i]) + ", ";
            }
            std::printf("[RANK-0], Vector Parcial es: [%s]\n", stra.c_str());

            vector_ordenado = merge(vector_ordenado, vector_parcial);
        }

        vector_ordenado = merge(vector_ordenado, vector_cero);

        std::string str3 = "";
        for (int i = 0; i < vector_ordenado.size(); i++) {
            str3 = str3 + std::to_string(vector_ordenado[i]) + ", ";
        }
        std::printf("[RANK-0], Vector Ordenado es: [%s]\n", str3.c_str());

    } else{
        int bloque;
        bool es_impar = vector.size() % 2 != 0;
        if (es_impar) {
            bloque = ((vector.size()) / nprocs) + 1;
        }else{
            bloque = (vector.size()) / nprocs;
        }

        std::vector<int> vector_tmp(bloque);//Soolo toca enviar el numero de elmentos por la particion

        //std::printf("Soy el rank %d recibiendo datos\n", rank);
        MPI_Recv(vector_tmp.data(), bloque, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //Concaternar en un String para imprimir los datos completos
        std::string str = "";
        for (int i = 0; i < bloque; i++) {
            str = str + std::to_string(vector_tmp[i]) + ", ";
        }
        std::printf("[RANK-%d] %d, datos recibidos: [%s]\n", rank, bloque, str.c_str());

        //-- 999. Operacion de los datos y enviar la operacion al Rank 0
        std::sort(vector_tmp.begin(), vector_tmp.end());
        MPI_Send(vector_tmp.data(), bloque,MPI_INT, 0, 0, MPI_COMM_WORLD);

    }

    //3. Finalizar
    MPI_Finalize();

    return 0;
}