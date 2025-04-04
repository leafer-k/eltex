#include <string.h>
#include <math.h>
#include <stdio.h>

float solve(char* expr, float (*operations[4])(float, float)){

    if (!checkExpr(expr)) return NAN;
    char temp[40];
    float subRes = 0;

    int lastSignIndex = 0, subStart = 0, subEnd = 0, firstOrderCnt = 0, secondOrderCnt = 0;

    if(expr[0] == '-') expr[0] = 'n';

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

	simpleSolve(temp, operations, &subRes);

	if (!isnan(subRes)) {
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
	simpleSolve(temp, operations, &subRes);
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

    float result = 0;
    int neg = 0;
    int frac = 0;
    float frac_div = 10.0f;

    int i = 0;
    if (expr[i] == '-') {
        neg = 1;
        i++;
    }

    for (; expr[i] != '\0'; i++) {
        if (expr[i] == '.') {
            frac = 1;
            continue;
        }
        if (!frac) {
            result = result * 10 + (expr[i] - '0');
        } else {
            result += (expr[i] - '0') / frac_div;
            frac_div *= 10.0f;
        }
    }

    return neg ? -result : result;
}
