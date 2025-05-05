#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFF_SIZE 1024
#define FILENAME_HEAD "filename:"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void parseFilenameSend(char* buff, char* dest) {
    strcpy(dest, FILENAME_HEAD);
    int header_len = strlen(FILENAME_HEAD);
    int filename_pos = 0;

    for (int j = 0; buff[j] != '\0'; j++) {
        if (buff[j] == '/') {
            filename_pos = j + 1;  // Пропускаем '/'
        }
    }

    int i = 0;
    while (buff[filename_pos] != '\0' && i < BUFF_SIZE - header_len - 1) {
        dest[header_len + i] = buff[filename_pos];
        filename_pos++;
        i++;
    }
    dest[header_len + i - 1] = '\0';
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	char buff[BUFF_SIZE];
	char filenameSend[BUFF_SIZE];
    printf("TCP CLIENT\n");

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

	portno = atoi(argv[2]);

	my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0)
        error("ERROR opening socket");

	server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // заполенние структуры serv_addr
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    // установка порта
	serv_addr.sin_port = htons(portno);

	// Шаг 2 - установка соединения
	if (connect(my_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

	// Шаг 3 - чтение и передача сообщений
	printf("While:\n");
	//while (1) {
		printf("Enter Filename\n");
		fgets(&buff[0], sizeof(buff) - 1, stdin);

		buff[strlen(buff) - 1] = '\0';

		int fd = open(buff, O_RDONLY);
	    if (fd == -1) {
    		perror("open");
			return 1;
    	}

		parseFilenameSend(buff, filenameSend);

		send(my_sock, filenameSend, strlen(filenameSend), 0);

		int sentLen = 0;

		while((sentLen = read(fd, buff, sizeof(buff))) > 0) {
			send(my_sock, &buff[0], sentLen, 0);
		}

		send(my_sock, 0, 0, 0);

        if (!strcmp(&buff[0], "quit\n"))
        {
            printf("\nExit...");
            close(my_sock);
            return 0;
        }

        // передаем строку клиента серверу
        send(my_sock, &buff[0], strlen(&buff[0]), 0);
    //}
//    printf("Recv error \n");
    close(my_sock);
    return -1;
}
