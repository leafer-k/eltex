#include <stdio.h>
#include <stdlib.h>

float main(int argc, char* argv[]) {
	float res = 0;
	for(int i = 1; i < argc; i++) {
		res += atof(argv[i]);
	}
	printf("Result: %g\n", res);
	return res;
}
