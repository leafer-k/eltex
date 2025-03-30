#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "headers.h"

int main(int argc, char* argv[]) {
    srand(time(NULL));
    char* mode;
    if(argc != 4 && argc != 5) {
	printf("Invalid arguments!\n");
	return 1;
    }

    char* gatewayStr = argv[1];
    char* maskStr = argv[2];
    char* numStr = argv[3];

    if(argc == 5) mode = argv[4];

    uint32_t gateway = strIPToInt(gatewayStr);
    uint32_t mask = strIPToInt(maskStr);
    int num = atoi(numStr);
    int res = 0;

     char buff[40];

    printf("Аргументы:\n\nАдрес шлюза: %s\nМаска подсети: %s\nN: %s\n\n", gatewayStr, maskStr, numStr);

    uint32_t addr;

    for(int i = 0; i < num; i++) {
	addr = generateAddr();
	if(checkSubnet(addr, gateway, mask)) {
	    res++;
	    if(argc == 5 && strlen(mode) == 1 && mode[0] == 's') {
		intIPToStr(addr, buff);
		printf("%s\n", buff);
	    }
	}
    }

    printf("Относится к подсети: %d/%d (%g%%)\n", res, num, ((float)res/num)*100);
    return 0;
}
