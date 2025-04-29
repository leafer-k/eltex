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

#define QUEUE_LEN 5
#define FILENAME_LEN 128
#define BUFF_LEN 1024
#define DOWNLOAD_FOLDER "./downloads/"

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
	int n = strlen(DOWNLOAD_FOLDER);
	for(; name[n] != '.' && n < strlen(name); n++);
	for(int i = strlen(name); i > n; i--) {
		name[i] = name[i-1];
	}
	name[strlen(name)] = '\0';
	name[n] = '1';

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

	#define str1 "Enter path to file\r\n"
	#define nan_str "NaN\r\n"
	#define filename_head "filename:"

   	write(sock, str1, sizeof(str1));

	strcpy(filepath, DOWNLOAD_FOLDER);

	bytes_recv = read(sock, &buff[0], sizeof(buff));

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

	long total_recieved = 0;

	while((bytes_recv = read(sock, &buff[0], sizeof(buff))) > 0) {
		bytes_written = write(fd, buff, bytes_recv);
		if(bytes_written == -1) {
			perror("write");
    		close(fd);
			break;
		}
		total_recieved += bytes_recv;
	}

	close(fd);

	printf("%ld bytes recieved\n", total_recieved);
	printf("File '%s' saved to ./downloads\n", filename);

	write(sock, buff, 0);

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
    // Шаг 1 - создание сокета
	// AF_INET     - сокет Интернета
    // SOCK_STREAM  - потоковый сокет (с
    //      установкой соединения)
    // 0 - по умолчанию выбирается TCP протокол
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // ошибка при создании сокета
	if (sockfd < 0)
       error("ERROR opening socket");

	// Шаг 2 - связывание сокета с локальным адресом
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // сервер принимает подключения на все IP-адреса
    serv_addr.sin_port = htons(portno);
    // вызываем bind для связывания
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
              error("ERROR on binding");
    // Шаг 3 - ожидание подключений, размер очереди - 5
	listen(sockfd, QUEUE_LEN);
    clilen = sizeof(cli_addr);

	// Шаг 4 - извлекаем сообщение из очереди
	// цикл извлечения запросов на подключение из очереди
	while (1) {
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
        	error("ERROR on accept");
		}

		nclients++; // увеличиваем счетчик
              		// подключившихся клиентов
		/*
		// вывод сведений о клиенте
		struct hostent *hst;
		hst = gethostbyaddr((char *)&cli_addr.sin_addr, 4, AF_INET);
		printf("+%s [%s] new connect!\n",
		(hst) ? hst->h_name : "Unknown host",
		(char*)inet_ntoa(cli_addr.sin_addr));*/
		printusers();

        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            clihandler(newsockfd);
            exit(0);
        } else {
			close(newsockfd);
		}
     }
     close(sockfd);
     return 0;
}
