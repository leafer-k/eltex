#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void parseArgs(char* in, char*** out) {
	int count = 1, temp = 0;
	int* argSizes;

	for(int i = 0; in[i] != '\0'; i++) {
		if(in[i] == ' ') {
			count++;
		}
	}

	int n = 0;

	(*out) = malloc(sizeof(char*) * (count + 1));
	argSizes = malloc(sizeof(int) * count + 1);

	if(!(*out) || !argSizes) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	int k = 0, j = 0;

	for(int i = 0; in[i] != '\0'; i++) {
		if(in[i] == ' ') {
			argSizes[k] = temp;
			temp = 0;
			k++;
			continue;
		}
		temp++;
	}

	argSizes[k+1] = temp;

	for(int i = 0; i < count; i++) {
		(*out)[i] = malloc(sizeof(char) * (argSizes[i] + 1));
		if(!(*out)[i]) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
	}

	k = 0;

	for(int i = 0; in[i] != '\0'; i++) {
		if(in[i] == ' ') {
			k++;
			(*out)[k][j] = '\0';
			j = 0;
			continue;
		}
		(*out)[k][j] = in[i];
		j++;
	}
	(*out)[k+1] = NULL;
	return;
}

int main(int argc, char* argv[]) {
	char buff[200];
	char** parsedArgs;
	pid_t child_pid;


	while(1) {
		printf(">> ");

        if(fgets(buff, sizeof(buff), stdin) == NULL) {
            break;
        }

        buff[strcspn(buff, "\n")] = '\0';

        if(strlen(buff) == 0) {
            continue;
        }

		if(strcmp(buff, "q") == 0) {
			exit(EXIT_SUCCESS);
		}

		child_pid = fork();

        if(child_pid == -1) {
                perror("fork");
                continue;
        }

		if(child_pid == 0) {
			parseArgs(buff, &parsedArgs);
			execv(parsedArgs[0], parsedArgs);
		} else {
			wait(NULL);
		}
	}
}
