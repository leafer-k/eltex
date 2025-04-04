#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <dlfcn.h>

#include "headers.h"

int main() {
    setlocale(LC_ALL, "Russian");

    void* handle;
    void (*menu)(struct List*);
    char* error;

    handle = dlopen("./liblist.so", RTLD_NOW);

    if (!handle) {
        fputs(dlerror(), stderr);
        return 1;
    }

    menu = dlsym(handle, "menu");

    if((error = dlerror()) != NULL) {
	fprintf(stderr, "%s\n", error);
	return 1;
    }

    struct List* list = (struct List*)malloc(sizeof(struct List));
    list->val = NULL;
    list->prev = NULL;
    list->next = NULL;


    (*menu)(list);

    dlclose(handle);

    return 0;
}
