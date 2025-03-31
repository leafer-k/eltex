#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "headers.h"

int main() {
    setlocale(LC_ALL, "Russian");
    struct List* list = (struct List*)malloc(sizeof(struct List));
    list->val = NULL;
    list->prev = NULL;
    list->next = NULL;


    menu(list);
    return 0;
}
