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
    int rows_per_proc;

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

    if (rows % size != 0) {
        if (rank == 0) {
            fprintf(stderr, "Number of rows must be divisible by the number of processes.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    rows_per_proc = rows / size;

    if (rank == 0) {
        // Rank 0 gera as matrizes
        matrix1 = (int *)malloc(rows * cols * sizeof(int));
        matrix2 = (int *)malloc(rows * cols * sizeof(int));
        result = (int *)malloc(rows * cols * sizeof(int));

        fill_matrix(matrix1, rows, cols);
        fill_matrix(matrix2, rows, cols);

        printf("Matrix 1:\n");
        print_matrix(matrix1, rows, cols);

        printf("Matrix 2:\n");
        print_matrix(matrix2, rows, cols);

        // Distribui as matrizes para os outros processos
        sub_matrix1 = (int *)malloc(rows_per_proc * cols * sizeof(int));
        sub_matrix2 = (int *)malloc(rows_per_proc * cols * sizeof(int));
        sub_result = (int *)malloc(rows_per_proc * cols * sizeof(int));

        MPI_Scatter(matrix1, rows_per_proc * cols, MPI_INT, sub_matrix1, rows_per_proc * cols, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(matrix2, rows_per_proc * cols, MPI_INT, sub_matrix2, rows_per_proc * cols, MPI_INT, 0, MPI_COMM_WORLD);

    } else {
        sub_matrix1 = (int *)malloc(rows_per_proc * cols * sizeof(int));
        sub_matrix2 = (int *)malloc(rows_per_proc * cols * sizeof(int));
        sub_result = (int *)malloc(rows_per_proc * cols * sizeof(int));

        MPI_Scatter(NULL, 0, MPI_INT, sub_matrix1, rows_per_proc * cols, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(NULL, 0, MPI_INT, sub_matrix2, rows_per_proc * cols, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Calcula a soma das sub-matrizes
    for (int i = 0; i < rows_per_proc * cols; ++i) {
        sub_result[i] = sub_matrix1[i] + sub_matrix2[i];
    }

    // Junta os resultados das sub-matrizes
    MPI_Gather(sub_result, rows_per_proc * cols, MPI_INT, result, rows_per_proc * cols, MPI_INT, 0, MPI_COMM_WORLD);

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

    MPI_Finalize();
    return EXIT_SUCCESS;
}