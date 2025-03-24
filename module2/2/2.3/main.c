#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include <string.h>

float sum(float a, float b) {
    return a+b;
}

float substract(float a, float b) {
    return a-b;
}

float multi(float a, float b) {
    return a*b;
}

float division(float a, float b) {
    return b == 0 ? NAN : a/b;
}

int isDigit(char c) {
    for(int j = 0; j < 10; j++){
	if(c == '0' + j) {
	    return 1;
	}
    }
    return 0;
}

int isDot(char c) {
    return c == '.';
}

int isSign(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

int checkExpr(char* expr){
    int dotFlag = 0, signFlag = 0, digitFlag = 0;

    for(int i = 0; expr[i] != '\0'; i++) {

        if((dotFlag && isDot(expr[i])) || (signFlag && isSign(expr[i]))) return 0;
        if(dotFlag == 0 && signFlag == 0 && digitFlag == 0 && i > 0) return 0;
        if(isSign(expr[i]) && digitFlag == 0) return 0;
        if((isSign(expr[i]) || isDot(expr[i])) && expr[i+1] == '\0') return 0;



        dotFlag = isDot(expr[i]);
        signFlag = isSign(expr[i]);
        digitFlag = isDigit(expr[i]);

}
    return 1;
}


int main() {
    setlocale(LC_ALL, "Russian");

    char expr[100];
    float (*operations[4])(float, float) = {sum, substract, multi, division};

    do {
        printf("Введите выражение. 0 - выход:\n");

        fgets(expr, sizeof(expr), stdin);
        expr[strcspn(expr, "\n")] = '\0';

        if (strcmp(expr, "0") == 0) break;

        printf("\nВы ввели: %s\n", expr);
        printf("Res: %d\n", checkExpr(expr));

    } while (1);

    return 0;
}
