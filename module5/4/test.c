#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define DEV_FILE_PATH "/dev/myDev"

int main(int argc, char* argv[]) {
	
	if(argc != 2) {
		printf("Use: %s <string>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	char buf[100];
	char i = 0;
	memset(buf, 0, 100);
	printf("Input: %s\n", argv[1]);

	int fp = open(DEV_FILE_PATH, O_RDWR);

	write(fp, argv[1], strlen(argv[1]));

	while(read(fp, &buf[i++], 1));

	printf("Driver: %s\n", buf);
	
	exit(EXIT_SUCCESS);
}
