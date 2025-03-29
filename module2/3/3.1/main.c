#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <sys/stat.h>

#include "headers.h"

int main() {
    setlocale(LC_ALL, "Russian");
    char* buff = malloc(sizeof(char) * 9);
    char input[100];
    struct stat st;

    while(1) {
	printf("\nВведите права доступа (в буквенном или цифровом обозначении), абсолютный путь к файлу или команду chmod. 0 - выход:\n");
	fgets(input, sizeof(input), stdin);
	input[strcspn(input, "\n")] = '\0';
	if(!strcmp(input, "0")) break;


	if(checkFormat(input) == -1) {
	    printf("Ошибка ввода!\n");
	    continue;
	} else {
	    switch(checkFormat(input)) {
		case 1:
		    if (stat(input, &st) != -1) {
			intToBinStr(st.st_mode, buff);
			printf("%s\n", buff);
			intToLetter(st.st_mode, buff);
			printf("%s\n", buff);
			intToOctStr(st.st_mode, buff);
			printf("%s\n", buff);
		    } else {
			printf("Не удалось найти файл!\n");
		    }
		    break;
		case 2:
		    int type = initType(input);
    		    if(type == -1) {
			printf("Invalid type!\n");
    		    } else {
			int mode = toBin(input, type);
			intToBinStr(mode, buff);
			printf("Битовое представление: %s\n", buff);
    		    }
		    break;
		case 3:
		    int res = changeMode(input);
		    if(res != -1) {
			printf("Новые права:\n");
			intToBinStr(res, buff);
			printf("%s\n", buff);
			intToLetter(res, buff);
			printf("%s\n", buff);
			intToOctStr(res, buff);
			printf("%s\n", buff);
		    } else {
			printf("Ошибка!\n");
		    }
		    break;
	    }
	}
    }

    return 0;
}
