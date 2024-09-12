#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void fill_matrix(int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i * cols + j] = rand() % 10; // Gera valores aleatórios entre 0 e 9
        }
    }
}

void print_matrix(int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int rows, cols;
    int *matrix1, *matrix2, *result;
    int *sub_matrix1, *sub_matrix2, *sub_result;
    int *sendcounts, *displs;
    int rows_per_proc, extra_rows, start_row, end_row;
    int total_elements, elements_per_proc;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <rows> <cols>\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    rows = atoi(argv[1]);
    cols = atoi(argv[2]);

    total_elements = rows * cols;

    // Calcular número de linhas e tamanho das sub-matrizes
    rows_per_proc = rows / size;
    extra_rows = rows % size;

    // Alocar memória para os buffers
    sendcounts = (int *)malloc(size * sizeof(int));
    displs = (int *)malloc(size * sizeof(int));

    // Definir o número de elementos a serem enviados para cada processo
    int offset = 0;
    for (int i = 0; i < size; ++i) {
        sendcounts[i] = (i < extra_rows) ? (rows_per_proc + 1) * cols : rows_per_proc * cols;
        displs[i] = offset;
        offset += sendcounts[i];
    }

    if (rank == 0) {
        matrix1 = (int *)malloc(total_elements * sizeof(int));
        matrix2 = (int *)malloc(total_elements * sizeof(int));
        result = (int *)malloc(total_elements * sizeof(int));

        fill_matrix(matrix1, rows, cols);
        fill_matrix(matrix2, rows, cols);

        printf("Matrix 1:\n");
        print_matrix(matrix1, rows, cols);

        printf("Matrix 2:\n");
        print_matrix(matrix2, rows, cols);
    }

    // Alocar memória para os buffers dos processos
    int sub_rows = (rank < extra_rows) ? (rows_per_proc + 1) : rows_per_proc;
    sub_matrix1 = (int *)malloc(sub_rows * cols * sizeof(int));
    sub_matrix2 = (int *)malloc(sub_rows * cols * sizeof(int));
    sub_result = (int *)malloc(sub_rows * cols * sizeof(int));

    // Distribuir as sub-matrizes usando MPI_Scatterv
    MPI_Scatterv(matrix1, sendcounts, displs, MPI_INT, sub_matrix1, sub_rows * cols, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(matrix2, sendcounts, displs, MPI_INT, sub_matrix2, sub_rows * cols, MPI_INT, 0, MPI_COMM_WORLD);

    // Calcular a soma das sub-matrizes
    for (int i = 0; i < sub_rows * cols; ++i) {
        sub_result[i] = sub_matrix1[i] + sub_matrix2[i];
    }

    // Coletar os resultados usando MPI_Gatherv
    MPI_Gatherv(sub_result, sub_rows * cols, MPI_INT, result, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result Matrix:\n");
        print_matrix(result, rows, cols);

        free(matrix1);
        free(matrix2);
        free(result);
    }

    free(sub_matrix1);
    free(sub_matrix2);
    free(sub_result);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return EXIT_SUCCESS;
}