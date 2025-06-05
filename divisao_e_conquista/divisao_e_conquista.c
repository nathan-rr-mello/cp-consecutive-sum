#include <mpi.h>

#define VETOR_SIZE 1000

int main(int argc, char **argv) {
    int my_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // pega pega o numero do processo atual (rank)

    int delta = 10;  // tamanho do vetor para ordenar diretamente
    int* vetor;
    int tam_vetor;
    int pai;

    if (my_rank != 0) {
        // recebo vetor
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &tam_vetor); // descubro tamanho da mensagem recebida
        MPI_Recv(&vetor, tam_vetor, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // não sou a raiz, tenho pai
    } else {
        tam_vetor = VETOR_SIZE;               // defino tamanho inicial do vetor
        inicializa(vetor, tam_vetor);      // sou a raiz e portanto gero o vetor - ordem reversa
    }

    // dividir ou conquistar?
    if (tam_vetor <= delta) {
        bubblesort(vetor, tam_vetor); // conquisto
    } else {
        int filho_esquerda = my_rank * 2 + 1; // filho esquerdo
        int filho_direita = my_rank * 2 + 2; // filho direito

        // dividir
        // quebrar em duas partes e mandar para os filhos
        int tam_esquerda = tam_vetor / 2; // tamanho da metade esquerda
        int tam_direita = tam_vetor - tam_esquerda; // tamanho da metade direita
        MPI_Send(&vetor[0], tam_esquerda, MPI_INT, filho_esquerda, 0, MPI_COMM_WORLD); // mando metade inicial do vetor
        MPI_Send(&vetor[tam_vetor/2], tam_direita, MPI_INT, filho_direita, 0, MPI_COMM_WORLD); // mando metade final do vetor

        // receber dos filhos
        MPI_Recv(&vetor[0], tam_esquerda, MPI_INT, filho_esquerda, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&vetor[tam_vetor/2], tam_direita, MPI_INT, filho_direita, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // intercalo vetor inteiro
        intercala(vetor, tam_vetor);
    }

    // mando para o pai
    if (my_rank != 0) {
        MPI_Send(vetor, tam_vetor, MPI_INT, pai, 0, MPI_COMM_WORLD); // tenho pai, retorno vetor ordenado pra ele
    } else {
        mostra(vetor); // sou o raiz, mostro vetor
    }

    MPI_Finalize();
}

void inicializa(int* vetor, int tam_vetor) {
    vetor = (int*) malloc(tam_vetor * sizeof(int));
    for (int i = 0; i < tam_vetor; i++) {
        vetor[i] = tam_vetor - i; // ordem reversa
    }
}

void bubblesort(int* vetor, int tam_vetor) {
    for (int i = 0; i < tam_vetor - 1; i++) {
        for (int j = 0; j < tam_vetor - i - 1; j++) {
            if (vetor[j] > vetor[j + 1]) {
                // troca
                int temp = vetor[j];
                vetor[j] = vetor[j + 1];
                vetor[j + 1] = temp;
            }
        }
    }
}

void intercala(int* vetor, int tam_vetor) {
    int i = 0, j = tam_vetor/2, k = 0;
    int tam_esquerda = tam_vetor / 2; // tamanho da metade esquerda
    int tam_direita = tam_vetor - tam_esquerda; // tamanho da metade direita
    int* result = (int*) malloc(tam_vetor * sizeof(int)); // vetor para armazenar o resultado

    // Enquanto houver elementos em ambos os arrays
    while (i < tam_esquerda && j < tam_direita) {
        if (vetor[i] <= vetor[j]) {
            result[k++] = vetor[i++];
        } else {
            result[k++] = vetor[j++];
        }
    }

    // Copia o restante do array A (se sobrou algo)
    while (i < tam_esquerda) {
        result[k++] = vetor[i++];
    }

    // Copia o restante do array B (se sobrou algo)
    while (j < tam_direita) {
        result[k++] = vetor[j++];
    }

    vetor = result;
}

void mostra(int* vetor) {
    printf("[");
    for (int i = 0; i < VETOR_SIZE; i++) {
        printf("%d ", vetor[i]);
    }
    printf("]\n");
}

