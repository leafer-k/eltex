#include <stdlib.h>
float (*getOperation(char c, float (*operations[4])(float, float)))(float, float) {
    float (*op)(float, float) = NULL;
    switch (c) {
        case '+': op = operations[0]; break;
        case '-': op = operations[1]; break;
        case '*': op = operations[2]; break;
        case '/': op = operations[3]; break;
	default: return NULL; break;
    }
    return op;
}
