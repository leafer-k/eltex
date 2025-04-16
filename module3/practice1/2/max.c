#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	if(argc == 1) {
		printf("Invalid args\n");
		exit(EXIT_FAILURE);
	}

	float max = atof(argv[1]);
	float cur;

	for(int i = 2; i < argc; i++) {
		cur = atof(argv[i]);
		max =  cur > max ? cur : max;
	}

	printf("Max: %g\n", max);
	return 0;
}
