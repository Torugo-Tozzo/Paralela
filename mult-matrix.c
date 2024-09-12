//Victor Hugo Tozzo Filho ----------------------  2020.1.08.018
//
//### Explicação:
//1. **Entrada de parâmetros**: O programa recebe as dimensões das matrizes como argumentos de linha de comando (`rows of matrix1`, `cols of matrix1/rows of matrix2`, `cols of matrix2`).
//2. **Distribuição do trabalho**: Cada processo recebe um subconjunto de linhas da primeira matriz. A segunda matriz é enviada inteira para todos os processos.
//3. **Multiplicação**: Cada processo realiza a multiplicação de suas linhas da primeira matriz pelas colunas da segunda matriz.
//4. **Coleta de resultados**: Os resultados parciais são enviados de volta ao processo raiz para formar a matriz final.
//5. **Impressão**: O processo raiz imprime a matriz resultado.
//
//Para compilar e rodar o programa:
//
//mpicc -o matrix_mult matrix_mult.c
//mpirun -np <n_procs> ./matrix_mult <rows1> <cols1> <cols2>
//
//Por exemplo, para multiplicar uma matriz 4x3 por uma 3x2 usando 4 processos:
//
//mpirun -np 4 ./matrix_mult 4 3 2
//
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
    int rows1, cols1, cols2;
    int *matrix1, *matrix2, *result;
    int *sub_matrix1, *sub_result;
    int rows_per_proc, extra_rows;
    int total_elements1, total_elements2, total_result_elements;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <rows of matrix1> <cols of matrix1/rows of matrix2> <cols of matrix2>\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    rows1 = atoi(argv[1]);  // Linhas da primeira matriz
    cols1 = atoi(argv[2]);  // Colunas da primeira matriz (que é igual às linhas da segunda matriz)
    cols2 = atoi(argv[3]);  // Colunas da segunda matriz

    total_elements1 = rows1 * cols1;
    total_elements2 = cols1 * cols2;
    total_result_elements = rows1 * cols2;

    rows_per_proc = rows1 / size;
    extra_rows = rows1 % size;

    if (rank == 0) {
        // Processo raiz gera as matrizes
        matrix1 = (int *)malloc(total_elements1 * sizeof(int));
        matrix2 = (int *)malloc(total_elements2 * sizeof(int));
        result = (int *)malloc(total_result_elements * sizeof(int));

        fill_matrix(matrix1, rows1, cols1);
        fill_matrix(matrix2, cols1, cols2);

        printf("Matrix 1:\n");
        print_matrix(matrix1, rows1, cols1);

        printf("Matrix 2:\n");
        print_matrix(matrix2, cols1, cols2);
    }

    // Alocar memória para sub-matriz e resultado local
    int sub_rows = (rank < extra_rows) ? (rows_per_proc + 1) : rows_per_proc;
    sub_matrix1 = (int *)malloc(sub_rows * cols1 * sizeof(int));
    sub_result = (int *)malloc(sub_rows * cols2 * sizeof(int));

    // Distribuir as linhas da primeira matriz entre os processos
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    int offset = 0;

    for (int i = 0; i < size; ++i) {
        sendcounts[i] = (i < extra_rows) ? (rows_per_proc + 1) * cols1 : rows_per_proc * cols1;
        displs[i] = offset;
        offset += sendcounts[i];
    }

    MPI_Scatterv(matrix1, sendcounts, displs, MPI_INT, sub_matrix1, sub_rows * cols1, MPI_INT, 0, MPI_COMM_WORLD);

    // Todos os processos recebem a segunda matriz
    if (rank != 0) {
        matrix2 = (int *)malloc(total_elements2 * sizeof(int));
    }
    MPI_Bcast(matrix2, total_elements2, MPI_INT, 0, MPI_COMM_WORLD);

    // Realizar a multiplicação de sub-matrizes
    for (int i = 0; i < sub_rows; ++i) {
        for (int j = 0; j < cols2; ++j) {
            sub_result[i * cols2 + j] = 0;
            for (int k = 0; k < cols1; ++k) {
                sub_result[i * cols2 + j] += sub_matrix1[i * cols1 + k] * matrix2[k * cols2 + j];
            }
        }
    }

    // Coletar os resultados dos processos
    int *recvcounts = (int *)malloc(size * sizeof(int));
    int *result_displs = (int *)malloc(size * sizeof(int));
    offset = 0;
    for (int i = 0; i < size; ++i) {
        recvcounts[i] = (i < extra_rows) ? (rows_per_proc + 1) * cols2 : rows_per_proc * cols2;
        result_displs[i] = offset;
        offset += recvcounts[i];
    }

    MPI_Gatherv(sub_result, sub_rows * cols2, MPI_INT, result, recvcounts, result_displs, MPI_INT, 0, MPI_COMM_WORLD);

    // Processo raiz imprime o resultado final
    if (rank == 0) {
        printf("Result Matrix:\n");
        print_matrix(result, rows1, cols2);

        free(matrix1);
        free(matrix2);
        free(result);
    }

    free(sub_matrix1);
    free(sub_result);
    free(sendcounts);
    free(displs);
    free(recvcounts);
    free(result_displs);

    MPI_Finalize();
    return EXIT_SUCCESS;
}