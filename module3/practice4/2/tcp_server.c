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
#include <math.h>

#define QUEUE_LEN 5

int isSign(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

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

float calc(float a, float b, char sign) {
	switch(sign) {
		case '+': return a+b; break;
		case '-': return a-b; break;
		case '*': return a*b; break;
		case '/':
			if(b != 0) {
				return a/b;
			} else {
				return NAN;
			}
			break;
	}
	return NAN;
}

void clihandler (int sock) {
	int bytes_recv;
	float a, b, res;
	char buff[20 * 1024];
	char sign;

	#define str1 "Enter 1st number\r\n"
	#define str2 "Enter 2nd number\r\n"
	#define str3 "Enter sign\r\n"
	#define sign_err_str "Invalid sign\r\n"
	#define nan_str "NaN\r\n"

   	write(sock, str1, sizeof(str1));

	bytes_recv = read(sock, &buff[0], sizeof(buff));
	if(bytes_recv < 0) error("ERROR reading from socket");
	a = atof(buff);

	write(sock, str2, sizeof(str2));

	bytes_recv = read(sock, &buff[0],sizeof(buff));
	if(bytes_recv < 0) error("ERROR reading from socket");

	b = atof(buff);

	write(sock, str3, sizeof(str3));

	bytes_recv = read(sock, &buff[0], sizeof(buff));
	if(bytes_recv < 0) error("ERROR reading from socket");

	if(!isSign(buff[0])) {
		write(sock, sign_err_str, sizeof(sign_err_str));
	} else {
		sign = buff[0];

		res = calc(a, b, sign);
		if(res == res) {
			snprintf(buff, sizeof(buff), "%g", res);
		} else {
			snprintf(buff, strlen(buff), nan_str);
		}

		buff[strlen(buff)] = '\n';

		write(sock, &buff[0], sizeof(buff));
	}

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
