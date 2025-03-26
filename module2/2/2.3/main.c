#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include "headers.h"

int main() {
    setlocale(LC_ALL, "Russian");

    char expr[100];

    do {
        printf("Введите выражение. 0 - выход:\n");

        fgets(expr, sizeof(expr), stdin);
        expr[strcspn(expr, "\n")] = '\0';

        if (strcmp(expr, "0") == 0) break;

        printf("Результат:\t%g\n\n", solve(expr));

    } while (1);

    return 0;
}
