#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "headers.h"

int main() {
    setlocale(LC_ALL, "Russian");
    Person** array = malloc(sizeof( Person*));
    array[0] = NULL;
    menu(&array);
    free(array);
    return 0;
}
