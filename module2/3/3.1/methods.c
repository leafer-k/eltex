#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "headers.h"

int powr(int a, int power){
    int res = a;
    for(int i = 1; i < power; i++) res *= a;
    return power == 0 ? 1 : res;
}

int isLetterType(char* c) {
    char letters[] = {'r', 'w', 'x'};

    for(int i = 0; c[i] != '\0'; i++) {
	if(i > 9) return 0;
	if(c[i] != letters[i%3] && c[i] != '-') return 0;
    }
    return 1;
}

int isDigitType(char* c) {
    for(int i = 0; c[i] != '\0'; i++) {
	if(i > 4) return 0;
	if(!(c[i] >= '0' && c[i] <= '7')) return 0;
    }
    return 1;
}

int initType(char* c) {
    if(isDigitType(c)) return 1;
    if(isLetterType(c)) return 2;
    return -1;
}

int letterToInt(char *c) {
    int res = 0, k = 0;
    for(int i = 8; i >= 0; i--) {
	res += (c[i] == '-' ? 0 : 1) * powr(2, k);
	k++;
    }
    return res;
}

int digitToInt(char* c) {
    int res = 0, k = 0;

    for(int i = 2; i >= 0; i--) {
	res += powr(8, k) * (c[i] - '0');
	k++;
    }

    return res;
}

int toBin(char* c, int type) {
    if(type == 1) {
	return digitToInt(c);
    }
    if(type == 2) {
	return letterToInt(c);
    }

    return 0;
}

void intToBinStr(int a, char* out) {
    for(int i = 8; i >= 0; i--){
	out[i] = a%2 + '0';
	a /= 2;
    }
    out[9] = '\0';
    return;
}

void intToLetter(int a, char* out) {
    char letters[] = "rwx";
    for(int i = 8; i >= 0; i--) {
	out[i] = a%2 == 1 ? letters[i%3] : '-';
	a/=2;
    }
    out[9] = '\0';
    return;
}

void intToOctStr(int a, char* out) {
    for(int i = 2; i >= 0; i--) {
	out[i] = a % 8 + '0';
	a/=8;
    }
    out[3] = '\0';
    return;
}

int changeMode(char* command) {
    int res, i = 0, k = 0;
    char path[100];
    char groups[5];
    char changeMode[5];
    int subMask = 0;
    int mask = 0;
    int oldMode, newMode;
    struct stat st;


    for(; command[i] != ' ' && command[i] != '\0'; i++);
    i++;
    for(k = 0; command[i] != '+' && command[i] != '-' && command[i] != '=' && command[i] != '\0'; i++, k++) groups[k] = command[i];
    groups[k] = '\0';
    for(k = 0; command[i] != ' ' && command[i] != '\0'; i++, k++) changeMode[k] = command[i];
    changeMode[k] = '\0';
    i++;
    for(k = 0; command[i] != ' ' && command[i] != '\0'; i++, k++) path[k] = command[i];
    path[k] ='\0';

    for(k = 1; changeMode[k] != '\0'; k++) {
	if(changeMode[k] == 'r') {
	    subMask += 4;
	} else if(changeMode[k] == 'w') {
	    subMask += 2;
	} else if (changeMode[k] == 'x') {
	    subMask += 1;
	} else {
	    printf("Error!\n");
	    return -1;
	}
    }

    for(k = 0; groups[k] != '\0'; k++) {
	if(groups[k] == 'u' || groups[k] == 'a') mask = (mask | (subMask << 6));
	if(groups[k] == 'g' || groups[k] == 'a') mask = (mask | (subMask << 3));
	if(groups[k] == 'o' || groups[k] == 'a') mask = mask | subMask;
    }

   if (stat(path, &st) != -1) {
	oldMode = st.st_mode;
	switch(changeMode[0]) {
	    case '+': newMode = oldMode | mask; break;
	    case '-': newMode = oldMode & (~mask); break;
	    case '=': newMode = mask; break;
	    default: return -1;
	}
    } else {
	printf("Error opening file!\n");
	return -1;
    }
    return newMode;
}

int checkFormat(char* str) {
    int flag = 0;
    int size = 0;
    char cmd[20];
    int i = 0;
    for(; str[i] != ' ' && str[i] != '\0'; i++) cmd[i] = str[i];
    cmd[i] = '\0';

    if(!strcmp(cmd, "chmod")) return 3;

    for(; str[size] != '\0'; size++);

    if(size < 3) return -1;

    if (str[0] == '/') return 1;
    for(int i = 0; str[i] != '\0'; i++) {
	if(str[i] != 'r' && str[i] != 'w' && str[i] != 'x' && str[i] != '-' || i == 9) {
	    flag = 1;
	    break;
	}
	if(i == 9) return -1;
    }

    if(flag == 0) return 2;

    for(int i = 0; str[i] != '\0'; i++) {
	if (i == 3 || str[i] - '0' < 0 || str[i] - '0' > 7) return -1;
    }

    return 2;
}
