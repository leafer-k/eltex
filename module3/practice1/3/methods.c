#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "headers.h"

#define MAX_PHONE_NUMS 5
#define MAX_EMAILS 5

void strCopy(char* dest, const char* src, size_t maxLen) {
    size_t i = 0;
    while (i < maxLen - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

size_t strLength(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strCompare(const char* str1, const char* str2) {
    while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) {
            return *str1 - *str2;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

void removeNewline(char* str) {
    size_t len = strLength(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void print(Person* p, int index) {
    printf("%d)\t%-15s %-15s %-15s %-20s\n",
        index, p->lastName, p->firstName, p->patronymic, p->company);

    printf("   Телефоны: ");
    int hasPhone = 0;
    for (int i = 0; i < MAX_PHONE_NUMS; i++) {
        if (p->phones[i]) {
            printf("%s ", p->phones[i]);
            hasPhone = 1;
        }
    }
    if (!hasPhone) printf("Нет");
    printf("\n");

    printf("   E-mails:  ");
    int hasEmail = 0;
    for (int i = 0; i < MAX_EMAILS; i++) {
        if (p->emails[i]) {
            printf("%s ", p->emails[i]);
            hasEmail = 1;
        }
    }
    if (!hasEmail) printf("Нет");
    printf("\n");
}

void printList(struct List* first) {
    struct List* list = first;
    int n = 0;

    if(first->val == NULL) {
	printf("Список пуст!\n");
	return;
    }

    while (list != NULL) {
        print(list->val, n);
        n++;
	list = list->next;
    }
}


void push(struct List** first, Person* new_person) {
    struct List* list = *first;

    if ((*first)->val == NULL) {
        (*first)->val = new_person;
        return;
    }

    struct List* new_node = (struct List*)malloc(sizeof(struct List));
    if (!new_node) {
        printf("Ошибка: не удалось выделить память\n");
        return;
    }

    new_node->val = new_person;

        while(list->next != NULL && strcmp(list->next->val->lastName, new_person->lastName) < 0) {
            list = list->next;
        }

	if(strcmp(list->val->lastName, new_person->lastName) > 0 && list == (*first)) {
	    new_node->next = *first;
	    new_node->prev = NULL;
	    (*first)->prev = new_node;
            *first = new_node;
	    return;
	}

        new_node->next = list->next;
	new_node->prev = list;
	if(new_node->next != NULL) new_node->next->prev = new_node;
        list->next = new_node;

    return;
}

void deleteIndex(struct List** first, int index) {
    if (!(*first) || index < 0) {
        printf("Invalid index\n");
        return;
    }

    struct List* list = *first;

    if(index == 0 && list->next == NULL) {
	free(list->val);
	list->val = NULL;
	return;
    }

    for (int i = 0; i < index; i++) {
        if (!list->next) {
            printf("Index out of range\n");
            return;
        }
        list = list->next;
    }

    if (list->prev) {
        list->prev->next = list->next;
    } else {
        *first = list->next;
    }

    if (list->next) {
        list->next->prev = list->prev;
    }

    free(list->val);
    free(list);
}



void initPerson(Person* new_p) {
    printf("Введите Фамилию: ");
    char temp[40];
    fgets(temp, 30, stdin);
    removeNewline(temp);
    strCopy(new_p->lastName, temp, 30);

    printf("Введите Имя: ");
    fgets(temp, 30, stdin);
    removeNewline(temp);
    strCopy(new_p->firstName, temp, 30);

    printf("Введите Отчество (если есть, иначе Enter): ");
    fgets(temp, 30, stdin);
    removeNewline(temp);
    strCopy(new_p->patronymic, temp, 30);

    printf("Введите Компанию (если есть, иначе Enter): ");
    fgets(temp, 30, stdin);
    removeNewline(temp);
    strCopy(new_p->company, temp, 30);

    for (int i = 0; i < MAX_PHONE_NUMS; i++) {
        printf("Введите телефон #%d (если нет, Enter): ", i + 1);
        fgets(temp, 15, stdin);
        removeNewline(temp);
        if (temp[0] == '\0') break;
        strCopy(new_p->phones[i], temp, strLength(temp) + 1);
    }

    for (int i = 0; i < MAX_EMAILS; i++) {
        printf("Введите e-mail #%d (если нет, Enter): ", i + 1);
        fgets(temp, 40, stdin);
        removeNewline(temp);
        if (temp[0] == '\0') break;
        strCopy(new_p->emails[i], temp, strLength(temp) + 1);
    }
    return;
}

void sort(struct List** list) {
    struct List* curr = *list;
    Person* tmp;

    while(curr->next != NULL) {
	if(strcmp(curr->next->val->lastName, curr->val->lastName) < 0) {
	    tmp = curr->val;
	    curr->val = curr->next->val;
	    curr->next->val = tmp;
	    curr = *list;
	    continue;
	}
	curr = curr->next;
    }
}


void addPerson(struct List** arr) {
    Person* new_p = malloc(sizeof( Person));
    if (!new_p) {
        perror("malloc failed");
        return;
    }
    for (int i = 0; i < sizeof(Person); i++) {
        ((char*)new_p)[i] = 0;
    }

    initPerson(new_p);
    push(arr, new_p);
}

void clearList(struct List** head) {
	while((*head)->next != NULL) {
		deleteIndex(head, 0);
	}

	free((*head)->val);
	(*head)->val = NULL;
	(*head)->next = NULL;
	return;
}

void editPerson(struct List** arr) {
    int index;
    printf("Введите индекс контакта для редактирования: ");
    scanf("%d", &index);
    getchar();

    if (index < 0) {
        printf("Invalid index\n");
        return;
    }

    struct List* list = *arr;

    for(int i = 0; i < index; i++) {
	list = list->next;
    }

     Person* p = list->val;

    int editChoice;
    do {
        printf("\nВыберите, что хотите изменить:\n");
        printf("1. Фамилия\n2. Имя\n3. Отчество\n4. Компания\n");
        printf("5. Телефоны\n6. E-mail\n0. Завершить редактирование\n");
        printf("Ваш выбор: ");
        scanf("%d", &editChoice);
        getchar();

        switch (editChoice) {
        case 1:
            printf("Введите новую фамилию (текущая: %s): ", p->lastName);
            fgets(p->lastName, 30, stdin);
            removeNewline(p->lastName);
            break;
        case 2:
            printf("Введите новое имя (текущее: %s): ", p->firstName);
            fgets(p->firstName, 30, stdin);
            removeNewline(p->firstName);
            break;
        case 3:
            printf("Введите новое отчество (текущее: %s): ", p->patronymic);
            fgets(p->patronymic, 30, stdin);
            removeNewline(p->patronymic);
            break;
        case 4:
            printf("Введите новую компанию (текущая: %s): ", p->company);
            fgets(p->company, 30, stdin);
            removeNewline(p->company);
            break;
        case 5:
            for (int i = 0; i < MAX_PHONE_NUMS; i++) {
                printf("Телефон #%d (текущий: %s, оставьте пустым, если не менять): ",
                    i + 1, p->phones[i] ? p->phones[i] : "нет");
                char temp[15];
		fgets(temp, 15, stdin);
                removeNewline(temp);
                if (temp[0] != '\0') {
                    strCopy(p->phones[i], temp, strLength(temp) + 1);
                }
            }
            break;
        case 6:
            for (int i = 0; i < MAX_EMAILS; i++) {
                printf("E-mail #%d (текущий: %s, оставьте пустым, если не менять): ",
                    i + 1, p->emails[i] ? p->emails[i] : "нет");
                char temp[40];
		fgets(temp, 40, stdin);
                removeNewline(temp);
                if (temp[0] != '\0') {
                    strCopy(p->emails[i], temp, strLength(temp) + 1);
                }
            }
            break;
        }
    } while (editChoice != 0);
    sort(arr);
}

void loadExample(struct List* list) {
     Person examples[] = {
        {"Иван", "Иванов", "Иванович", "TechCorp", {"89005553322", "89261112233"}, {"ivan@corp.com", "ivan123@gmail.com"}},
        {"Мария", "Сидорова", "Александровна", "SoftGroup", {"89037778899"}, {"maria@soft.com"}},
        {"Андрей", "Козлов", "Викторович", "ITSolutions", {"89161114455", "89997776655"}, {"andrey.k@it.com", "kozlov.a@gmail.com"}},
        {"Ольга", "Петрова", "Николаевна", "DevSoft", {"89051234567"}, {"olga@dev.com", "petrova.olya@mail.ru"}}
    };

    int num_examples = sizeof(examples) / sizeof(examples[0]);
    for (int i = 0; i < num_examples; i++) {
	Person* newPerson = (Person*)malloc(sizeof(Person));
        *newPerson = examples[i];
	push(&list, newPerson);
    }

    printf("Примерные контакты загружены!\n");
}

int loadFromFile(char* filename, struct List** head) {
	struct List* list = *head;
	int fd = open(filename, O_RDONLY);
	int size = 0;
	char* buff;
	size_t readBytes;

	if(fd == -1) {
		perror("File open");
		return -1;
	}

	clearList(head);

	readBytes = read(fd, &size, sizeof(int));

	if(readBytes != sizeof(int)) {
		perror("Read size");
		close(fd);
		return -1;
	}

	for(int i = 0; i < size; i++) {
		Person* new_p = malloc(sizeof( Person));
    	if (!new_p) {
	        perror("malloc failed");
			close(fd);
        	return -1;
    	}
		readBytes = read(fd, new_p, sizeof(Person));
		if(readBytes != sizeof(Person) || !new_p) {
			perror("Read Person");
			close(fd);
			return -1;
		}
		push(head, new_p);
	}

	return 0;
}

int saveToFile(char* filename, struct List** head) {
	struct List* list = *head;

	if(list->val == NULL) {
		printf("List empty\n");
		return -1;
	}

	int fd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
	int size = 0;
	size_t written;

	if(fd == -1) {
		perror("File open");
		return -1;
	}

	while(list != NULL) {
		size++;
		list = list->next;
	}

	if (size) {
		write(fd, &size, sizeof(int));
	} else {
		printf("Error. Size null");
		close(fd);
		return -1;
	}
	list = *head;

	while(list != NULL) {
		written = write(fd, list->val, sizeof(Person));
		if(written != sizeof(Person)) {
			perror("Write");
			close(fd);
			return -1;
		}
		list = list->next;
	}

	close(fd);

	return 0;
}

void menu(struct List* list) {
    int choice;
    do {
        printf("\n1. Вывести список\n2. Добавить контакт\n3. Удалить контакт\n4. Редактировать\n5. Загрузить пример\n6. Сохранить в файл\n7. Загрузить из файла\n8. Очистить список\n0. Выйти\n");
        scanf("%d", &choice);
        getchar();
		char buff[100];
        switch (choice) {
        case 1: printList(list); break;
        case 2: addPerson(&list); break;
        case 3: {
            int delIndex;
            printf("Введите индекс для удаления: ");
            scanf("%d", &delIndex);
            deleteIndex(&list, delIndex);
            break;
        }
        case 4: editPerson(&list); break;
        case 5: loadExample(list); break;
		case 6: {
			printf("Введите имя файла для сохранения:\n");
			scanf("%s", &buff);
			if(saveToFile(buff, &list) == 0) {
				printf("Сохранено в файл %s\n", buff);
			}
			break;
		}
		case 7: {
			printf("Введите имя файла для загрузки:\n");
			scanf("%s", &buff);
			if(loadFromFile(buff, &list) == 0) {
				printf("Загружено\n");
			}
			break;
		}
		case 8: clearList(&list); break;
	default: break;
        }
    } while (choice != 0);
}
