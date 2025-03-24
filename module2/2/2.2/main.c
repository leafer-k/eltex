#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>

float sum(float a, float b) {
    return a+b;
}

float diff(float a, float b) {
    return a-b;
}

float multi(float a, float b) {
    return a*b;
}

float division(float a, float b) {
    return b == 0 ? NAN : a/b;
}


int main() {
    setlocale(LC_ALL, "Russian");

    char action;
    float a, b, res = NAN;
    do {
	res = NAN;
	a = NAN;
	b = NAN;
	printf("Введите действие (+ - * /). 0 - выход:\n");

	scanf(" %c", &action);
	if(action == '0') break;

	printf("Введите 2 числа:\n");
        scanf("%f %f", &a, &b);

	if(b == 0 && action == '/') {
	    printf("\nDivision by zero\n");
	    continue;
	}

	switch (action) {
	    case '+': res = sum(a, b); break;
	    case '-': res = diff(a, b); break;
	    case '*': res = multi(a, b); break;
	    case '/': res = division(a, b); break;
	    default: printf("\nWrong input\n"); continue;
	}

	printf("\n%g %c %g = %g\n\n", a, action, b, res);

    } while (action != '0');


    return 0;
}
