#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MASTER_RANK 0
#define TAG_DATA 0
#define TAG_RESULT 1

int main(int argc, char** argv) {
    int rank, size;
    int num_values = 1000 + rand() % 1001; // Gera um número aleatório entre 1000 e 2000
    int* values;
    int sum = 0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == MASTER_RANK) {
        srand(time(NULL));
        
        // Mestre gera os valores aleatórios
        values = (int*)malloc(num_values * sizeof(int));
        for (int i = 0; i < num_values; i++) {
            values[i] = rand() % 100;
        }
        
        // Mestre envia o número de valores para o escravo
        for (int slave_rank = 1; slave_rank < size; slave_rank++) {
            MPI_Send(&num_values, 1, MPI_INT, slave_rank, TAG_DATA, MPI_COMM_WORLD);
        }
        
        // Mestre envia os valores para o escravo
        for (int slave_rank = 1; slave_rank < size; slave_rank++) {
            MPI_Send(values, num_values, MPI_INT, slave_rank, TAG_DATA, MPI_COMM_WORLD);
        }
        
        // Mestre recebe os resultados dos escravos e calcula a soma total
        for (int slave_rank = 1; slave_rank < size; slave_rank++) {
            int partial_sum;
            MPI_Recv(&partial_sum, 1, MPI_INT, slave_rank, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += partial_sum;
        }
        
        printf("Soma total: %d\n", sum);
        free(values);
    } else {
        // Escravo recebe o número de valores
        MPI_Recv(&num_values, 1, MPI_INT, MASTER_RANK, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        values = (int*)malloc(num_values * sizeof(int));
        
        // Escravo recebe os valores
        MPI_Recv(values, num_values, MPI_INT, MASTER_RANK, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Escravo calcula a soma parcial
        int partial_sum = 0;
        for (int i = 0; i < num_values; i++) {
            partial_sum += values[i];
        }
        
        // Escravo envia o resultado de volta para o mestre
        MPI_Send(&partial_sum, 1, MPI_INT, MASTER_RANK, TAG_RESULT, MPI_COMM_WORLD);
        
        free(values);
    }
    
    MPI_Finalize();
    return 0;
}
