#include <stdio.h>
#include <stdlib.h>
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

void print( Person* p, int index) {
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

void printArr( Person** arr) {
    int size = 0;
    while (arr[size] != NULL) {
        print(arr[size], size);
        size++;
    }
}

void push( Person*** arr,  Person* new_person) {
    int size = 0;
    while ((*arr)[size] != NULL) size++;

     Person** new_arr = realloc(*arr, (size + 2) * sizeof( Person*));
    if (!new_arr) {
        perror("realloc failed");
        return;
    }

    new_arr[size] = malloc(sizeof( Person));
    if (!new_arr[size]) {
        perror("malloc failed");
        return;
    }

    for (int i = 0; i < sizeof( Person); i++) {
        ((char*)new_arr[size])[i] = ((char*)new_person)[i];
    }

    new_arr[size + 1] = NULL;
    *arr = new_arr;
}

void deleteIndex( Person*** arr, int index) {
    int size = 0;
    while ((*arr)[size] != NULL) size++;

    if (index < 0 || index >= size) {
        printf("Index out of range\n");
        return;
    }

    free((*arr)[index]);

    for (int i = index; i < size - 1; i++) {
        (*arr)[i] = (*arr)[i + 1];
    }

    (*arr)[size - 1] = NULL;
}


void initPerson( Person* new_p) {
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
        new_p->phones[i] = malloc((strLength(temp) + 1) * sizeof(char));
        strCopy(new_p->phones[i], temp, strLength(temp) + 1);
    }

    for (int i = 0; i < MAX_EMAILS; i++) {
        printf("Введите e-mail #%d (если нет, Enter): ", i + 1);
        fgets(temp, 40, stdin);
        removeNewline(temp);
        if (temp[0] == '\0') break;
        new_p->emails[i] = malloc((strLength(temp) + 1) * sizeof(char));
        strCopy(new_p->emails[i], temp, strLength(temp) + 1);
    }
    return;
}




void addPerson( Person*** arr) {
     Person* new_p = malloc(sizeof( Person));
    if (!new_p) {
        perror("malloc failed");
        return;
    }
    for (int i = 0; i < sizeof( Person); i++) {
        ((char*)new_p)[i] = 0;
    }

    initPerson(new_p);

    push(arr, new_p);
}

void editPerson( Person** arr) {
    int index;
    printf("Введите индекс контакта для редактирования: ");
    scanf("%d", &index);
    getchar();

    if (arr[index] == NULL) {
        printf("Invalid index\n");
        return;
    }

     Person* p = arr[index];

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
                    if (p->phones[i]) free(p->phones[i]);
                    p->phones[i] = malloc((strLength(temp) + 1) * sizeof(char));
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
                    if (p->emails[i]) free(p->emails[i]);
                    p->emails[i] = malloc((strLength(temp) + 1) * sizeof(char));
                    strCopy(p->emails[i], temp, strLength(temp) + 1);
                }
            }
            break;
        }
    } while (editChoice != 0);
}

void loadExample( Person*** arr) {
     Person examples[] = {
        {"Иван", "Иванов", "Иванович", "TechCorp", {"89005553322", "89261112233"}, {"ivan@corp.com", "ivan123@gmail.com"}},
        {"Мария", "Сидорова", "Александровна", "SoftGroup", {"89037778899"}, {"maria@soft.com"}},
        {"Андрей", "Козлов", "Викторович", "ITSolutions", {"89161114455", "89997776655"}, {"andrey.k@it.com", "kozlov.a@gmail.com"}},
        {"Ольга", "Петрова", "Николаевна", "DevSoft", {"89051234567"}, {"olga@dev.com", "petrova.olya@mail.ru"}}
    };

    int num_examples = sizeof(examples) / sizeof(examples[0]);
    for (int i = 0; i < num_examples; i++) {
        push(arr, &examples[i]);
    }

    printf("Примерные контакты загружены!\n");
}


void menu( Person*** arr) {
    int choice;
    do {
        printf("\n1. Вывести список\n2. Добавить контакт\n3. Удалить контакт\n4. Редактировать\n5. Загрузить пример\n0. Выйти\n");
        scanf("%d", &choice);
        getchar();
        switch (choice) {
        case 1: printArr(*arr); break;
        case 2: addPerson(arr); break;
        case 3: {
            int delIndex;
            printf("Введите индекс для удаления: ");
            scanf("%d", &delIndex);
            deleteIndex(arr, delIndex);
            break;
        }
        case 4: editPerson(*arr); break;
        case 5: loadExample(arr);
	default: break;
        }
    } while (choice != 0);
}
