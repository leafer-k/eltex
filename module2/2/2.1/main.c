#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

struct Person {
    int id;
    char firstName[30];
    char patronymic[30];
    char lastName[30];
    char company[30];
    char emails[15][40];
    char otherContacts[15][80];
};

int menu(struct Person* arr) {
    return 0;
}

void print(struct Person* p) {

    printf("%d\t%s %s %s\t%s\n", p->id, p->lastName, p->firstName, p->patronymic, p->company);
    if (p->emails[0][0] != '\0') {
        printf("Emails:\n");
        for (int i = 0; i < sizeof(p->emails) / sizeof(p->emails[1]); i++) {
            printf("%s\n", p->emails[i]);
        }
    }
    if (p->otherContacts[0][0] != '\0') {
        printf("Other contacts:\n");
        for (int i = 0; i < sizeof(p->otherContacts) / sizeof(p->otherContacts[1]); i++) {
            printf("%s\n", p->otherContacts[i]);
        }
    }
}

void printArr(struct Person** arr) {
    for (int i = 0; arr[i] != NULL; i++) {
        print(arr[i]);
    }
}

void push(struct Person*** arr, struct Person* new_person) {
    int size = 0;
    while ((*arr)[size] != NULL) {
        size++;
    }

    struct Person** new_arr = realloc(*arr, (size + 2) * sizeof(struct Person*));
    if (!new_arr) {
        perror("realloc failed");
        return;
    }

    new_arr[size] = new_person;
    new_arr[size + 1] = NULL;
    *arr = new_arr;
}



int main() {
    struct Person** array = malloc(sizeof(struct Person*));  
    if (!array) {
        perror("malloc failed");
        return 1;
    }
    array[0] = NULL;

    // Создаем новых людей и добавляем их в массив
    struct Person p1 = { 1, "Александр", "Сергеевич", "Коровин", "Comp" };
    struct Person p2 = { 2, "Евгений", "Петрович", "Сидоров", "TechCorp" };
    struct Person p3 = { 3, "Мария", "Викторовна", "Зайцева", "ITSoft" };
    struct Person p4 = { 4, "Дмитрий", "Александрович", "Морозов", "DevTeam" };
    struct Person p5 = { 5, "Ольга", "Николаевна", "Смирнова", "SoftLab" };

    push(&array, &p1);
    push(&array, &p2);
    push(&array, &p3);
    push(&array, &p4);
    push(&array, &p5);

    printArr(array);

    free(array);

    return 0;
}
