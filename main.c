#include <stdio.h>

int contar_somas_consecutivas(int n) {
    int count = 0;
    for (int k = 1; k * (k + 1) / 2 <= n; k++) {
        int numerador = n - k * (k - 1) / 2;
        if (numerador % k == 0) {
            count++;
        }
    }
    return count;
}

void encontrar_maior_representacao(int limite) {
    int max_count = 0;
    int numero_com_mais_formas = 0;

    for (int i = 1; i <= limite; i++) {
        int formas = contar_somas_consecutivas(i);
        if (formas > max_count) {
            max_count = formas;
            numero_com_mais_formas = i;
        }
    }

    printf("Número com mais representações entre 1 e %d: %d\n", limite, numero_com_mais_formas);
    printf("Total de formas: %d\n", max_count);
}

int main() {
    int limite = 25000000; //25_000_000
    encontrar_maior_representacao(limite);
    return 0;
}