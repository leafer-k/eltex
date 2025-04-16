#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//Возвращает номер аргумента, который является наибольшей строкой

int main(int argc, char* argv[]) {
	int maxIndex = 0, maxLen;

	if(argc <= 1) {
		printf("Invalig arguments\n");
		return -1;
	}

	maxLen = strlen(argv[1]);

	for(int i = 2; i < argc; i++) {
		if(strlen(argv[i]) > maxLen) {
			maxIndex = i - 1;
			maxLen = strlen(argv[i]);
		}
	}

	printf("Max length: %d\nString: %s\n", maxLen, argv[maxIndex+1]);
	return maxIndex;
}


