#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include <dlfcn.h>

int main() {
    setlocale(LC_ALL, "Russian");

    float (*operations[4])(float, float);
    float (*solve)(char *, float (*[4])(float, float));
    void* temp;

    char libs[12][60] =
		{"libclrsp.so",
		"libdiv.so",
		"libgetop.so",
		"libisdig.so",
		"libisdot.so",
		"libissign.so",
		"libcheckexp.so",
		"libmulti.so",
		"libsimslv.so",
		"libsolve.so",
		"libsubstract.so",
		"libsum.so"};

    char expr[100];

    void* handles[12];
    char* error;
    char path[100];

    for(int i = 0; i < 12; i++) {
	snprintf(path, sizeof(path), "./libs/%s", libs[i]);
	handles[i] = dlopen(path, RTLD_NOW | RTLD_GLOBAL);

	if((error = dlerror()) != NULL) {
            fprintf(stderr, "%s\n", error);
            return 1;
	}
    }

    for(int i = 0; i < 12; i++) {
	temp = dlsym(handles[i], "solve");
	if (temp) solve = (float (*)(char*, float (*[4])(float, float)))temp;

	temp = dlsym(handles[i], "sum");
	if (temp) operations[0] = (float (*)(float, float))temp;

	temp = dlsym(handles[i], "substract");
	if (temp) operations[1] = (float (*)(float, float))temp;

	temp = dlsym(handles[i], "multi");
	if (temp) operations[2] = (float (*)(float, float))temp;

	temp = dlsym(handles[i], "division");
	if (temp) operations[3] = (float (*)(float, float))temp;
    }

    for(int i = 0; i < 4; i++) {
	if(!operations[i]) {
	    fprintf(stderr, "Операция не загружена\n");
	    return 1;
	}
    }

    if (!solve) {
	fprintf(stderr, "solve() не загружена\n");
	return 1;
    }


    do {
        printf("Введите выражение. 0 - выход:\n");

        fgets(expr, sizeof(expr), stdin);
        expr[strcspn(expr, "\n")] = '\0';

        if (strcmp(expr, "0") == 0) break;

        printf("Результат:\t%g\n\n", (*solve)(expr, operations));

    } while (1);


    for(int i = 0; i < 12; i++) {
	dlclose(handles[i]);
    }

    return 0;
}
