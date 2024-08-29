#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ESCRAVOS 4 // Número de processos escravos
#define TAMANHO_BLOCO 10000 // Aumentar o tamanho do bloco para tornar o trabalho mais demorado
#define NUM_BLOCO_TOTAL 5 * (NUM_ESCRAVOS + 1) // Número total de blocos a serem gerados (M vezes o número de processos)

// Função para gerar um bloco de dados
void gerar_dados(float *bloco, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        bloco[i] = (float)(rand() % 1000) / 10; // Números aleatórios entre 0.0 e 99.9
    }
}

int main(int argc, char **argv) {
    int rank, size;
    float *dados = NULL;
    int blocos_por_escravo;
    int num_blocos = NUM_BLOCO_TOTAL / NUM_ESCRAVOS;
    float soma_total = 0;
    float soma_local;
    int i;

    // Inicializar o MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Definir a semente aleatória baseada no tempo
    srand(time(NULL));

    if (rank == 0) {
        // Mestre
        dados = (float*)malloc(NUM_BLOCO_TOTAL * TAMANHO_BLOCO * sizeof(float));
        if (dados == NULL) {
            fprintf(stderr, "Erro ao alocar memória.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Gerar blocos de dados
        for (i = 0; i < NUM_BLOCO_TOTAL; i++) {
            gerar_dados(dados + i * TAMANHO_BLOCO, TAMANHO_BLOCO);
        }

        // Enviar blocos para os escravos
        for (i = 1; i < size; i++) {
            for (int j = 0; j < num_blocos; j++) {
                MPI_Send(dados + j * TAMANHO_BLOCO, TAMANHO_BLOCO, MPI_FLOAT, i, j, MPI_COMM_WORLD);
            }
        }

        // Receber somas dos escravos
        for (i = 1; i < size; i++) {
            for (int j = 0; j < num_blocos; j++) {
                MPI_Recv(&soma_local, 1, MPI_FLOAT, i, j, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Recebido do escravo %d, bloco %d: soma local = %f\n", i, j, soma_local);
                soma_total += soma_local;
            }
        }

        // Calcular a soma dos dados restantes do mestre (se houver)
        soma_local = 0;
        for (i = 0; i < TAMANHO_BLOCO; i++) {
            soma_local += dados[i];
        }
        soma_total += soma_local;

        printf("Soma total: %f\n", soma_total);

        free(dados);
    } else {
        // Escravos
        float *bloco = (float*)malloc(TAMANHO_BLOCO * sizeof(float));
        if (bloco == NULL) {
            fprintf(stderr, "Erro ao alocar memória.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        for (int j = 0; j < num_blocos; j++) {
            // Receber bloco de dados do mestre
            MPI_Recv(bloco, TAMANHO_BLOCO, MPI_FLOAT, 0, j, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Calcular soma local
            soma_local = 0;
            for (i = 0; i < TAMANHO_BLOCO; i++) {
                soma_local += bloco[i];
            }

            // Enviar soma local de volta ao mestre
            MPI_Send(&soma_local, 1, MPI_FLOAT, 0, j, MPI_COMM_WORLD);
        }

        free(bloco);
    }

    MPI_Finalize();
    return 0;
}
