#include <stdio.h>
#include <omp.h>
#include <mpi.h>
#include <string.h>

typedef struct {
    int numero;
    int somas;
} Resultado;

enum Tag {
    TRABALHO = 50,
    TERMINAR = 51
};

Resultado contar_somas_consecutivas(int n) {
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

    #pragma omp parallel for schedule(guided)
    for (int i = inicio; i <= fim; i++) {
        Resultado res = contar_somas_consecutivas(i);
        #pragma omp critical 
        {
            if (res.somas > maior_resultado.somas) {
                maior_resultado = res;
            }
        }
    }

    return maior_resultado;
}

void rajada_inicial(int proc_n, int limite, int grao) {

    int tag = 50;
    for (int i = 1; i < proc_n; i++) {
        //int inicio = (limite / proc_n) * i + 1;
        int inicio = (i - 1) * grao + 1;
        //int fim = (limite / proc_n) * (i + 1);
        int fim = i * grao;
        if (fim > limite) {
            fim = limite;
        }
        int range[2] = {inicio, fim};
        MPI_Send(range, 2, MPI_INT, i, tag, MPI_COMM_WORLD);
    }
}

void realiza_trabalho() {
    
    while (1) {

        int source = 0;
        int range[2];
        MPI_Status status; // estrutura que guarda o estado de retorno
        MPI_Recv(range, 2, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG != TERMINAR) {
            int inicio = range[0];
            int fim = range[1];

            Resultado resultado = encontrar_maior_representacao_trivial(inicio, fim);
            int maior_resultado[2] = {resultado.numero, resultado.somas};
            MPI_Send(maior_resultado, 2, MPI_INT, source, TRABALHO, MPI_COMM_WORLD);
        } else {
            break;
        }
    }
}

void envia_trabalho(int proc_n, int limite, int grao) {

    Resultado maior_resultado = {0, 0};

    int terminaram = 0;
    int inicio = (proc_n - 1) * grao + 1; // começa depois da primeira rajada
    for (int i = inicio; terminaram < proc_n - 1; i += grao) {
        // recebe resultado
        int resposta[2];
        MPI_Status status; // estrutura que guarda o estado de retorno
        MPI_Recv(resposta, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        Resultado resultado = {resposta[0], resposta[1]};
        if (resultado.somas > maior_resultado.somas) {
            maior_resultado = resultado;
		}

        // se houver trabalho
        if (i < limite) {
            // envia mais trabalho
            int fim = i + grao - 1;
            if (fim > limite) {
                fim = limite;
            }
            int range[2] = {i, fim};
            MPI_Send(range, 2, MPI_INT, status.MPI_SOURCE, TRABALHO, MPI_COMM_WORLD);
        } else {
            // avisa que acabou
            //MPI_Send(NULL, 0, MPI_DATATYPE_NULL, status.MPI_SOURCE, TERMINAR, MPI_COMM_WORLD);
            int vazio[2] = {0, 0};
            MPI_Send(vazio, 2, MPI_INT, status.MPI_SOURCE, TERMINAR, MPI_COMM_WORLD);
            terminaram++;
        }
    }

    printf("Número com mais representações entre 1 e %d: %d\n", limite, maior_resultado.numero);
    printf("Total de formas: %d\n", maior_resultado.somas);
}

int main(int argc, char** argv)
{
    int limite = 1000000; // 25_000_000
    //int limite = 9;
    int my_rank;       // Identificador deste processo
    int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)
    int message;       // Buffer para as mensagens
    MPI_Status status; // estrutura que guarda o estado de retorno

    int grao = 100; // o tamanho do range que será enviado

    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)
    char coordenador[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(coordenador, &name_len);
    MPI_Bcast(coordenador, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, MPI_COMM_WORLD);

    char my_hostname[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name(my_hostname, &name_len);

    double t1,t2;
    t1 = MPI_Wtime();  // inicia a contagem do tempo

    if (my_rank == 0) {
        omp_set_num_threads(7);
        double start = omp_get_wtime();
        rajada_inicial(proc_n, limite, grao);
        envia_trabalho(proc_n, limite, grao);
        
        double end = omp_get_wtime();
        printf("Nó 0 - Work took %f seconds\n", end - start);

        t2 = MPI_Wtime(); // termina a contagem do tempo
        printf("\nTempo de execucao total: %f\n\n", t2-t1);
    } else {
        omp_set_num_threads(8);
        if (strcmp(coordenador, my_hostname) == 0) {
            omp_set_num_threads(7);
        }
        double start = omp_get_wtime();
        realiza_trabalho();
        double end = omp_get_wtime();
        printf("Nó  %d - Work took %f seconds\n", my_rank, end - start);
    }

    MPI_Finalize(); // funcao que finaliza o MPI, todo o codigo paralelo estah acima
}
