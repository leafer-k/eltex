#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include <string.h>

#include "headers.h"

float (*operations[4])(float, float) = {sum, substract, multi, division};

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

float (*getOperation(char c))(float, float) {
    switch (c) {
	case '+': return operations[0]; break;
	case '-': return operations[1]; break;
	case '*': return operations[2]; break;
	case '/': return operations[3]; break;
	default: return NULL; break;
    }
    return NULL;
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
    int dotFlag = 0, signFlag = 0, digitFlag = 0, dotDigitFlag = 0;
    int i = 0;

    if(expr[0] != '-' && isDigit(expr[0]) == 0) return 0;

    if(expr[0] == '-') {
	expr[0] = 'n';
	i = 1;
    }

    for(; expr[i] != '\0'; i++) {
	if(isSign(expr[i])) dotDigitFlag = 0;
        if((dotFlag && isDot(expr[i])) || (signFlag && isSign(expr[i]))) return 0;
	if(!(expr[0] == 'n' && i == 1)) { if(dotFlag == 0 && signFlag == 0 && digitFlag == 0 && i > 0) return 0; }
        if(isSign(expr[i]) && digitFlag == 0) return 0;
        if((isSign(expr[i]) || isDot(expr[i])) && expr[i+1] == '\0') return 0;
	if(dotDigitFlag == 1 && isDot(expr[i])) return 0;

	if(dotFlag == 1 && isDigit(expr[i])) dotDigitFlag = 1;

        dotFlag = isDot(expr[i]);
        signFlag = isSign(expr[i]);
        digitFlag = isDigit(expr[i]);

    }
    return 1;
}



float simpleSolve(char* expr) {
    char charNums[2][20];
    char sign;

    int i = 0, k = 0, j = 0, neg = 0;
    int dotFlag = 0;

    if(expr[0] == 'n') {
	neg = 1;
	i++;
    }

    for(; expr[i] != '\0'; i++) {
	if(isSign(expr[i])) {
	    sign = expr[i];
	    k++;
	    j = 0;
	    continue;
	}
	charNums[k][j] = expr[i];
	j++;
    }

    float num1, num2;

    num1 = atof(charNums[0]);
    num2 = atof(charNums[1]);

    if(neg == 1) num1 *= -1;

    return getOperation(sign)(num1, num2);
}


void clearSpaces(char* str) {
    int spaces = 0, start = 0;
    for(int i = 0; str[i] != '\0'; i++) {
	if(str[i] == ' ') {
	    spaces++;
	    if (start == 0) start = i;
	}
    }

    for(int i = start; ; i++) {
	str[i] = str[i+spaces];
	if(str[i] == '\0') break;
    }
}


float solve(char* expr){
    if (!checkExpr(expr)) return NAN;
    char temp[30];
    float subRes;

    int lastSignIndex = 0, subStart = 0, subEnd = 0, firstOrderCnt = 0, secondOrderCnt = 0;

    if(expr[0] == '-') expr[0] == 'n';

    for(int i = 0; expr[i] != '\0'; i++) {
	if(expr[i] == '*' || expr[i] == '/') {
	    firstOrderCnt++;
	} else if (expr[i] == '+' || expr[i] == '-') {
	    secondOrderCnt++;
	}
    }

    int k = 0;

    for(int j = 0; j < firstOrderCnt; j++) {
	for(int i = 0; expr[i] != '\0'; i++) {
	    if(expr[i] == '-' || expr[i] == '+') {
		 lastSignIndex = i;
	    } else if(expr[i] == '*' || expr[i] == '/') {
		i++;
		while (expr[i] != '\0' && !isSign(expr[i])) i++;
		subEnd = i;
		break;
	    }
	}
	subStart = lastSignIndex == 0 ? 0 : lastSignIndex + 1;

	for(int i = subStart; i < subEnd; i++) {
	    temp[k] = expr[i];
	    expr[i] = ' ';
	    k++;
	}
	temp[k] = '\0';

	subRes = simpleSolve(temp);
	if (subRes != NAN) {
	    snprintf(temp, sizeof temp, "%g", subRes);
	} else {
	    return NAN;
	}

	int resultLen = strlen(temp);
        int spaceAvailable = subEnd - subStart;
        if (resultLen > spaceAvailable) {
            memmove(&expr[subStart + resultLen], &expr[subEnd], strlen(&expr[subEnd]) + 1);
        }

	k = 0;

	for(int m = subStart; temp[k] != '\0'; m++) {
	    expr[m] = temp[k];
	    k++;
	}

	clearSpaces(expr);

	k = 0;
	subStart = 0;
	subEnd = 0;
	lastSignIndex = 0;
    }

    int signCount = 0;

    for(int j = 0; j < secondOrderCnt; j++) {
	signCount = 0;
	for(int i = 0; expr[i] != '\0'; i++){
	    if(isSign(expr[i])) signCount++;
	    if(signCount == 2 || expr[i+1] == '\0') {
		subEnd = i;
		break;
	    }
	}

	k = 0;

	if(signCount != 2) subEnd++;

	for(; k < subEnd; k++) {
	    temp[k] = expr[k];
	    expr[k] = ' ';
	}

	temp[k] = '\0';

	subRes = simpleSolve(temp);
	if (subRes != NAN) {
	    snprintf(temp, sizeof temp, "%g", subRes);
	} else {
	    return NAN;
	}

	if(temp[0] == '-') temp[0] = 'n';

        for(int m = 0; temp[m] != '\0'; m++) expr[m] = temp[m];
        clearSpaces(expr);

	subEnd = 0;
    }

    if(expr[0] == 'n') expr[0] = '-';
    return atof(expr);
}
