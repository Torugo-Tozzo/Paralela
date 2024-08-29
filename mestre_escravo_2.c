#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h> // Adicionado para a função time

#define MASTER_RANK 0
#define TAG_DATA 0
#define TAG_RESULT 1
#define TAG_DONE 2
#define CHUNK_SIZE 30 // Tamanho dos blocos de valores enviados

int main(int argc, char** argv) {
    int rank, size;
    int* values = NULL;
    int* sub_values = NULL;
    int count;
    int partial_sum;
    int total_sum = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Inicializa o gerador de números aleatórios com a semente baseada no tempo
    if (rank == MASTER_RANK) {
        srand(time(NULL));
    }

    if (rank == MASTER_RANK) {
        int num_values = 1000; // Total de valores a serem processados
        int remaining_values = num_values;
        int start_index = 0;

        // Aloca espaço para todos os valores (para o mestre)
        values = (int*)malloc(num_values * sizeof(int));
        for (int i = 0; i < num_values; i++) {
            values[i] = rand() % 100; // Gera valores aleatórios
        }

        // Envia blocos iniciais para os escravos
        for (int i = 1; i < size; i++) {
            int chunk_size = (remaining_values < CHUNK_SIZE) ? remaining_values : CHUNK_SIZE;
            MPI_Send(&chunk_size, 1, MPI_INT, i, TAG_DATA, MPI_COMM_WORLD);
            MPI_Send(values + start_index, chunk_size, MPI_INT, i, TAG_DATA, MPI_COMM_WORLD);
            start_index += chunk_size;
            remaining_values -= chunk_size;
        }

        // Envia tarefas adicionais enquanto há valores restantes
        while (remaining_values > 0) {
            // Recebe resultados dos escravos
            MPI_Recv(&partial_sum, 1, MPI_INT, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
            int source_rank = status.MPI_SOURCE; // Rank do escravo que enviou o resultado
            total_sum += partial_sum;
            
            // Exibe o resultado recebido e de qual escravo veio
            printf("Recebido %d do escravo %d\n", partial_sum, source_rank);

            // Prepara novos dados para enviar ao escravo que acabou de enviar o resultado
            int chunk_size = (remaining_values < CHUNK_SIZE) ? remaining_values : CHUNK_SIZE;
            MPI_Send(&chunk_size, 1, MPI_INT, source_rank, TAG_DATA, MPI_COMM_WORLD);
            MPI_Send(values + start_index, chunk_size, MPI_INT, source_rank, TAG_DATA, MPI_COMM_WORLD);
            start_index += chunk_size;
            remaining_values -= chunk_size;
        }

        // Envia sinais de término para os escravos
        for (int i = 1; i < size; i++) {
            int zero_chunk = 0;
            MPI_Send(&zero_chunk, 1, MPI_INT, i, TAG_DONE, MPI_COMM_WORLD);
        }

        // Recebe as somas finais dos processos
        for (int i = 1; i < size; i++) {
            MPI_Recv(&partial_sum, 1, MPI_INT, i, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_sum += partial_sum;
        }

        printf("Soma total: %d\n", total_sum);
        free(values);
    } else {
        while (1) {
            // Recebe o número de valores e os dados
            MPI_Recv(&count, 1, MPI_INT, MASTER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_DONE) break; // Finaliza quando o mestre enviar o sinal de término

            sub_values = (int*)malloc(count * sizeof(int));
            MPI_Recv(sub_values, count, MPI_INT, MASTER_RANK, TAG_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Calcula a soma parcial
            partial_sum = 0;
            for (int i = 0; i < count; i++) {
                partial_sum += sub_values[i];
            }

            // Envia a soma parcial de volta para o mestre
            MPI_Send(&partial_sum, 1, MPI_INT, MASTER_RANK, TAG_RESULT, MPI_COMM_WORLD);

            free(sub_values);
        }
    }

    MPI_Finalize();
    return 0;
}
