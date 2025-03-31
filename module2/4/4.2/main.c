#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "headers.h"


int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(NULL));

    struct List* list = (struct List*)malloc(sizeof(struct List));
    list->val = NULL;
    list->next = NULL;

    menu(list);

    return 0;
}
