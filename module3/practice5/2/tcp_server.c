/* Пример простого TCP сервера
   Порт является аргументом, для запуска сервера неободимо ввести:
   # ./[имя_скомпилированного_файла] [номер порта]
   # ./server 57123
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <errno.h>

#define QUEUE_LEN 20
#define FILENAME_LEN 128
#define BUFF_LEN 1024
#define DOWNLOAD_FOLDER "./downloads/"
#define EOF_STR "EOF"

// Функция обработки ошибок
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int nclients = 0;

void printusers() {
	if(nclients)
	{printf("%d user on-line\n",nclients);}
	else {printf("No User on line\n");}
}

void parseFilename(char* buff, char* dest, int len) {
	int n = 0, i = 0;
	for(; buff[n] != ':'; n++);
	n++;
	for(; n < len && i < FILENAME_LEN; i++, n++) {
		dest[i] = buff[n];
	}
	dest[i] = '\0';
	return;
}


void addOneToName(char* name) {
    char *dot = strrchr(name, '.');
    int insert_pos;

    if (dot != NULL) {
        insert_pos = dot - name;
    } else {
        insert_pos = strlen(name);
    }

    for (int i = strlen(name) + 1; i > insert_pos; i--) {
        name[i] = name[i-1];
    }
    name[insert_pos] = '1';
    name[strlen(name) + 1] = '\0';
}


int file_exists (char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

void clihandler (int sock) {
	int bytes_recv;
	char buff[BUFF_LEN];
	char filename[FILENAME_LEN];
	char filepath[FILENAME_LEN + strlen(DOWNLOAD_FOLDER)];
	char sign;
	int bytes_written;
	long total_recieved = 0;

	#define str1 "Enter path to file\r\n"
	#define nan_str "NaN\r\n"
	#define filename_head "filename:"

   	write(sock, str1, sizeof(str1));

	strcpy(filepath, DOWNLOAD_FOLDER);

	bytes_recv = read(sock, &buff[0], sizeof(buff));
	if(bytes_recv != 0) {
		if(strncmp(&buff[0], filename_head, strlen(filename_head)) == 0) {
			parseFilename(buff, &filename, bytes_recv);
		}

		strcat(filepath, filename);

		while(file_exists(filepath)) {
			addOneToName(filepath);
		}

		int fd = open(filepath, O_WRONLY | O_CREAT, 0644);
		if (fd == -1) {
		    perror("open");
		    return;
		}

		while((bytes_recv = read(sock, &buff[0], sizeof(buff))) != -1) {
			if(strncmp(buff, EOF_STR, strlen(EOF_STR)) == 0) {
				break;
			}
			bytes_written = write(fd, buff, bytes_recv);
			if(bytes_written == -1) {
				perror("write");
	    		close(fd);
				break;
			}
			total_recieved += bytes_recv;
		}
		close(fd);
		printf("File '%s' saved to ./downloads\n", filename);
	}

	printf("%ld bytes recieved\n", total_recieved);

//	write(sock, buff, 0);

	nclients--;
    printf("-disconnect\n");
	printusers();
	return;
}


int main(int argc, char *argv[])
{
    char buff[1024];
	printf("TCP SERVER DEMO\n");

	int sockfd, newsockfd; // дескрипторы сокетов
	int portno; // номер порта
	int pid; // id номер потока
    socklen_t clilen; // размер адреса клиента типа socklen_t
    struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента

	// ошибка в случае если мы не указали порт
    if (argc < 2)
	{
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
       error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
              error("ERROR on binding");
	listen(sockfd, QUEUE_LEN);
    clilen = sizeof(cli_addr);

	fd_set clifds;
	int client_sockets[QUEUE_LEN] = {0};
    int max_sd, activity;

    while (1) {
        FD_ZERO(&clifds);
        FD_SET(sockfd, &clifds);
        max_sd = sockfd;

        for (int i = 0; i < QUEUE_LEN; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &clifds);
                if (client_sockets[i] > max_sd) max_sd = client_sockets[i];
            }
        }

        activity = select(max_sd + 1, &clifds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select");
        }

        if (FD_ISSET(sockfd, &clifds)) {
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0) {
                perror("accept");
                continue;
            }

            printf("New connection, socket fd: %d\n", newsockfd);
            nclients++;
            printusers();

            for (int i = 0; i < QUEUE_LEN; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = newsockfd;
                    break;
                }
            }
        }

        for (int i = 0; i < QUEUE_LEN; i++) {
            int sd = client_sockets[i];
            if (sd > 0 && FD_ISSET(sd, &clifds)) {
                clihandler(sd);
                client_sockets[i] = 0;
            }
        }
    }

     close(sockfd);
     return 0;
}
