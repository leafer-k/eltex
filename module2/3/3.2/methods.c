#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "headers.h"

int powr(int a, int power){
    int res = a;
    for(int i = 1; i < power; i++) res *= a;
    return power == 0 ? 1 : res;
}

uint32_t strIPToInt(char* str) {
    uint32_t res = 0;
    int j = 0;
    uint32_t tmp[4] = {0, 0, 0, 0};
    for(int i = 0; str[i] != '\0'; i++) {
	if(str[i] == '.') {
	    j++;
	    continue;
	}

	tmp[j] += str[i] - '0';
	tmp[j] *= 10;
    }

    for(int i = 0; i < 4; i++) {
	tmp[i] /= 10;
	tmp[i] = tmp[i] << 8*(3-i);
	res = res | tmp[i];
    }

    return res;
}

void intIPToStr(uint32_t addr, char* out) {
    int tmp, tmpRes = 0;
    int k = 0;
    char tmpc;
    for(int i = 0; i < 4; i++) {
	tmp = addr & 255;
	for(int j = 0; tmp > 0; j++) {
	    tmpRes += tmp % 2 * powr(2, j);
	    tmp /= 2;
	}

	if(tmpRes == 0) {
	    out[k] = '0';
	    k++;
	}
	for(int j = 0; tmpRes > 0; j++) {
	    out[k] = tmpRes % 10 + '0';
	    tmpRes /= 10;
	    k++;
	}
	out[k] = '.';
	k++;
	addr = addr >> 8;
    }
    k--;
    out[k] = '\0';
    k--;

    for(int i = 0; i <= (k / 2); i++) {
	tmpc = out[i];
	out[i] = out[k - i];
	out[k - i] = tmpc;
    }
    return;
}

uint32_t generateAddr() {
    uint32_t res = 0;

    for(int i = 0; i < 4; i++) {
	res += (rand() % 256) << (8 * i);
    }

    return res;
}

int checkSubnet(uint32_t addr, uint32_t gateway, uint32_t mask) {
    if((addr & mask) == (gateway & mask)) return 1;
    return 0;
}
