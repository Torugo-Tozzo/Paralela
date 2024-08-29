#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MASTER_RANK 0
#define TAG_DATA 0
#define TAG_RESULT 1

int main(int argc, char** argv) {
    int rank, size;
    int num_values;
    int* values = NULL;
    int* sub_values = NULL;
    int* recv_counts = NULL;
    int* displs = NULL;
    int sum = 0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Definindo a semente aleatória diferente para cada processo
    srand(time(NULL) + rank);
    
    if (rank == MASTER_RANK) {
        // Mestre gera o número aleatório de valores
        num_values = 1000 + rand() % 1001; // Gera um número aleatório entre 1000 e 2000
        
        // Mestre gera os valores aleatórios
        values = (int*)malloc(num_values * sizeof(int));
        for (int i = 0; i < num_values; i++) {
            values[i] = rand() % 100;
        }
        
        // Preparar os dados para dispersão
        recv_counts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        int base_count = num_values / size;
        int extra = num_values % size;
        
        for (int i = 0; i < size; i++) {
            recv_counts[i] = base_count + (i < extra ? 1 : 0);
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recv_counts[i - 1];
        }
        
        // Enviar os dados para todos os processos
        for (int i = 1; i < size; i++) {
            MPI_Send(&recv_counts[i], 1, MPI_INT, i, TAG_DATA, MPI_COMM_WORLD);
            MPI_Send(values + displs[i], recv_counts[i], MPI_INT, i, TAG_DATA, MPI_COMM_WORLD);
        }
        
        // Mestre processa sua própria parte
        int master_partial_sum = 0;
        for (int i = 0; i < recv_counts[MASTER_RANK]; i++) {
            master_partial_sum += values[i];
        }
        
        sum += master_partial_sum;
        
        // Mestre recebe os resultados dos escravos e calcula a soma total
        for (int i = 1; i < size; i++) {
            int partial_sum;
            MPI_Recv(&partial_sum, 1, MPI_INT, i, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("recebendo %d, do escravo %d\n", partial_sum, i);
            sum += partial_sum;
        }
        
        printf("Soma total: %d\n", sum);
        
        free(values);
        free(recv_counts);
        free(displs);
    } else {
        // Escravo recebe o número de valores e os dados
        int count;
        MPI_Recv(&count, 1, MPI_INT, MASTER_RANK, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        sub_values = (int*)malloc(count * sizeof(int));
        MPI_Recv(sub_values, count, MPI_INT, MASTER_RANK, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Escravo calcula a soma parcial
        int partial_sum = 0;
        for (int i = 0; i < count; i++) {
            partial_sum += sub_values[i];
        }
        
        // Escravo envia o resultado de volta para o mestre
        MPI_Send(&partial_sum, 1, MPI_INT, MASTER_RANK, TAG_RESULT, MPI_COMM_WORLD);
        
        free(sub_values);
    }
    
    MPI_Finalize();
    return 0;
}
