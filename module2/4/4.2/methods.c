#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <stdint.h>

#include "headers.h"

int curr_id = 0;

void printTask(Task* task) {
    printf("%d\t%d\t%s\n", task->id, task->priority, task->name);
    return;
}

void printList(struct List* first) {
    struct List* list = first;

    if(list->val == NULL) {
	printf("Очередь пуста\n");
	return;
    }

    while(list != NULL) {
	printTask(list->val);
	list = list->next;
    }
    return;
}


void add(struct List** first, Task* new_task) {
    struct List* list = *first;

    if(list->val == NULL) {
	list->val = new_task;
	return;
    }

    struct List* new_node = (struct List*)malloc(sizeof(struct List));
    new_node->val = new_task;

    if(list->val->priority <= new_task->priority) {
	while(list->next != NULL && list->next->val->priority <= new_task->priority) {
	    list = list->next;
	}
	new_node->next = list->next;
	list->next = new_node;
    } else {
	new_node->next = *first;
	*first = new_node;
    }

    return;
}



Task* initTask(int priority, char* name) {
    Task* new_task = (Task*)malloc(sizeof(Task));
    new_task->id = curr_id;
    new_task->priority = priority;
    curr_id++;

    char* new_name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(new_name, name);

    new_task->name = new_name;

    return new_task;
}


int pullTaskByPriority(struct List** list, int pr){
    struct List** curr = list;
    struct List** prev = list;

    while(curr != NULL) {
	if((*curr)->val->priority == pr) {
	    removeTask(prev, curr);
	    return 1;
	}
	prev = curr;
	curr = &((*curr)->next);
    }
    return 0;
}

void removeTask(struct List** prev, struct List** curr) {
    if((*prev) == (*curr)) {
	(*curr) = (*curr)->next;
    } else {
	(*prev)->next = (*curr)->next;
    }
    return;
}

void pullTasksHigherPriority(struct List** list, int pr) {
    struct List** curr = list;
    struct List** prev = curr;
    if((*curr)->val == NULL) return;

    while((*curr)->next != NULL && (*curr)->val->priority <= pr) {
	removeTask(prev, curr);
    }
    return;
}

void generateTasks(struct List** list, int num) {
    for (int i = 0; i < num; i++) {
        int priority = rand() % (MAX_PRI + 1);

        char name[20];
        snprintf(name, sizeof(name), "task%d", curr_id);

        add(list, initTask(priority, name));
    }
}

void pullFirst(struct List** list) {
    removeTask(list, list);
}


void menu(struct List* list) {
    char action;
    char buff[100];
    int pr, n;

    do {
	printf("\n1. Добавить задачу\n2. Вывести очередь\n3. Выполнить первую задачу из очереди\n4. Выполнение задачи по приоритету\n5. Выполнение задач с приоритетом, не ниже заданного\n6. Сгенерировать пример\n0. Выход\n\nВведите команду:\n");
	scanf(" %c", &action);

	switch(action) {
	    case '1':
		printf("Введите название:\n");

		scanf("%99s", buff);

		printf("Введите приоритет:\n");
		scanf("%d", &pr);

		if(pr < 0 || pr > MAX_PRI) {
		    printf("Ошибка!\n");
		    break;
		}

		add(&list, initTask(pr, buff));
		break;
	    case '2':
		printf("id\tPri\tName\n");
		printList(list);
		break;
	    case '3':
		if(list->val) {
		    pullFirst(&list);
		} else {
		    printf("Очередь пуста!\n");
		}
		break;
	    case '4':
		printf("Введите приоритет:\n");
		scanf("%d", &pr);
		if(pullTaskByPriority(&list, pr)) {
		    printf("Выполнено\n");
		} else {
		    printf("Не найдено\n");
		}
		break;
	    case '5':
		printf("Введите приоритет:\n");
		scanf("%d", &pr);
		pullTasksHigherPriority(&list, pr);
		break;
	    case '6':
		printf("Введите количество:\n");
		scanf("%d", &n);
		generateTasks(&list, n);
		printf("Добавлено %d задач\n", n);
		break;
	    default: break;
	}

    } while(action != '0');

}
