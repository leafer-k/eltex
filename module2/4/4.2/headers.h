#define MAX_PRI 255

typedef struct {
    int id;
    uint8_t priority;
    char* name;
} Task;

struct List {
    Task* val;
    struct List* next;
};

void printTask(Task*);
void printList(struct List*);
void add(struct List**, Task*);
void removeTask(struct List**, struct List**);

Task* initTask(int, char*);

int pullTaskByPriority(struct List**, int);

void pullTasksHigherPriority(struct List**, int);

void generateTasks(struct List**, int);

void pullFirst(struct List**);

void menu(struct List*);
