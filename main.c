#include <stdio.h>
#include <omp.h>

typedef struct {
    int numero;
    int somas;
} Resultado;

// Declara a redução customizada para comparar e manter o melhor Resultado.
#pragma omp declare reduction(max_result : Resultado : \
    omp_out = omp_in.somas > omp_out.somas ? omp_in : omp_out ) \
    initializer(omp_priv = omp_orig)

Resultado contar_somas_consecutivas(int n) {
    int count = 0;
    for (int k = 2; k * (k - 1) / 2 < n; k++) {
        int numerador = n - k * (k - 1) / 2;
        if (numerador % k == 0) {
            count++;
        }
    }

    Resultado res = { .numero = n, .somas = count };

    return res;
}

void encontrar_maior_representacao(int limite) {
    Resultado maior_resultado = {0,0};

    #pragma omp parallel for schedule(guided) reduction(max_result:maior_resultado)
    for (int i = 1; i <= limite; i++) {
        Resultado res = contar_somas_consecutivas(i);
        maior_resultado = res.somas > maior_resultado.somas ? res : maior_resultado;
    }

    printf("Número com mais representações entre 1 e %d: %d\n", limite, maior_resultado.numero);
    printf("Total de formas: %d\n", maior_resultado.somas);
}

void encontrar_maior_representacao_trivial(int limite) {

    Resultado maior_resultado = {0,0};

    #pragma omp parallel for schedule(guided)
    for (int i = 1; i <= limite; i++) {
        Resultado res = contar_somas_consecutivas(i);
        #pragma omp critical 
        {
            if (res.somas > maior_resultado.somas) {
                maior_resultado = res;
            }
        }
    }

    printf("Número com mais representações entre 1 e %d: %d\n", limite, maior_resultado.numero);
    printf("Total de formas: %d\n", maior_resultado.somas);
}

int main() {
    int limite = 25000000; //25_000_000
    //int limite = 9;
    omp_set_num_threads(12);
    double start = omp_get_wtime();
    encontrar_maior_representacao(limite);
    double end = omp_get_wtime();
    printf("Work took %f seconds\n", end - start);
}