#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "headers.h"

int main() {
    setlocale(LC_ALL, "Russian");
    struct Person** array = malloc(sizeof(struct Person*));
    array[0] = NULL;
    menu(&array);
    free(array);
    return 0;
}
