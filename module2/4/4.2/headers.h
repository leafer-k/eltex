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

Task* initTask(int, char*);
Task* getTaskByPriority(struct List*, int);

void printTasksHigherPriority(struct List*, int);

void generateTasks(struct List**, int);

Task* getFirst(struct List*);

void menu(struct List*);
