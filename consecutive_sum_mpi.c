#include <stdio.h>
#include "mpi.h"

typedef struct
{
    int numero;
    int somas;
} Resultado;

Resultado contar_somas_consecutivas(int n)
{
    int count = 0;
    for (int k = 2; k * (k - 1) / 2 < n; k++)
    {
        int numerador = n - k * (k - 1) / 2;
        if (numerador % k == 0)
        {
            count++;
        }
    }

    Resultado res = {.numero = n, .somas = count};

    return res;
}

Resultado encontrar_maior_representacao_trivial(int inicio, int fim) {

    Resultado maior_resultado = {0, 0};

    for (int i = inicio; i <= fim; i++) {
        Resultado res = contar_somas_consecutivas(i);
        if (res.somas > maior_resultado.somas) {
            maior_resultado = res;
        }
    }

    // printf("Número com mais representações entre 1 e %d: %d\n", limite, maior_resultado.numero);
    // printf("Total de formas: %d\n", maior_resultado.somas);
    return maior_resultado;
}

void separa_trabalho(int proc_n, int limite) {

    int tag = 50;
    for (int i = 0; i < proc_n; i++) {
        int inicio = (limite / proc_n) * i + 1;
        int fim = (limite / proc_n) * (i + 1);
        if (i == proc_n - 1) {
            fim = limite;
        }
        int range[2] = {inicio, fim};
        MPI_Send(range, 2, MPI_INT, i + 1, tag, MPI_COMM_WORLD);
    }
}

void realiza_trabalho() {

    int source = 0;
    int tag = 50;
    int range[2];
    MPI_Status status; // estrutura que guarda o estado de retorno
    MPI_Recv(range, 2, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
    int inicio = range[0];
    int fim = range[1];

    Resultado resultado = encontrar_maior_representacao_trivial(inicio, fim);
    int maior_resultado[2] = {resultado.numero, resultado.somas};
    MPI_Send(maior_resultado, 2, MPI_INT, source, tag, MPI_COMM_WORLD);
}

void finaliza_trabalho(int proc_n, int limite) {

    Resultado maior_resultado = {0, 0};
    int tag = 50;
    MPI_Status status; // estrutura que guarda o estado de retorno
    for (int i = 1; i <= proc_n; i++) {
        int resposta[2];
        MPI_Recv(resposta, 2, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
        Resultado resultado = {resposta[0], resposta[1]};
        if (resultado.somas > maior_resultado.somas) {
            maior_resultado = resultado;
        }
    }

    printf("Número com mais representações entre 1 e %d: %d\n", limite, maior_resultado.numero);
    printf("Total de formas: %d\n", maior_resultado.somas);
}

int main(int argc, char** argv)
{
    int limite = 1000; // 25_000_000
    // int limite = 9;
    int my_rank;       // Identificador deste processo
    int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)
    int message;       // Buffer para as mensagens
    MPI_Status status; // estrutura que guarda o estado de retorno

    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

    if (my_rank == 0) {
        separa_trabalho(proc_n, limite);
        finaliza_trabalho(proc_n, limite);
    } else {
        realiza_trabalho();
    }

    MPI_Finalize(); // funcao que finaliza o MPI, todo o codigo paralelo estah acima
}