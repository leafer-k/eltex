#include "libgetop.h"

void simpleSolve(char* expr, float (*operations[4])(float, float), float* resLink) {
    char charNums[2][20];
    memset(charNums, 0, sizeof(charNums));
    memset(charNums[0], 0, sizeof(charNums[0]));
    memset(charNums[1], 0, sizeof(charNums[1]));

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
	    charNums[k][j] = '\0';
	    k++;
	    j = 0;
	    continue;
	}
	charNums[k][j] = expr[i];
	j++;
	charNums[k][j] = '\0';
    }

    float num1 = 0, num2 = 0;
    int frac = 0;
    float frac_div = 10.0f;

    for (j = 0; charNums[0][j] != '\0'; j++) {
        if (charNums[0][j] == '.') {
            frac = 1;
            continue;
        }
        if (!frac) {
            num1 = num1 * 10 + (charNums[0][j] - '0');
        } else {
            num1 += (charNums[0][j] - '0') / frac_div;
            frac_div *= 10.0f;
        }
    }

    frac = 0;
    frac_div = 10.0f;
    for (j = 0; charNums[1][j] != '\0'; j++) {
        if (charNums[1][j] == '.') {
            frac = 1;
            continue;
        }
        if (!frac) {
            num2 = num2 * 10 + (charNums[1][j] - '0');
        } else {
            num2 += (charNums[1][j] - '0') / frac_div;
            frac_div *= 10.0f;
        }
    }


    if(neg == 1) num1 *= -1;

    float result = getOperation(sign, operations)(num1, num2);

    *resLink = result;
    return;
}
