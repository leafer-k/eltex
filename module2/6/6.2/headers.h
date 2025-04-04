#define MAX_PHONE_NUMS 5
#define MAX_EMAILS 5

typedef struct {
    char firstName[30];
    char lastName[30];
    char patronymic[30];
    char company[30];
    char* phones[MAX_PHONE_NUMS];
    char* emails[MAX_EMAILS];
} Person;

struct List {
    struct List* prev;
    struct List* next;
    Person* val;
};

void strCopy(char*, const char*, size_t);
size_t strLength(const char*);
int strCompare(const char*, const char*);
void removeNewline(char*);

void print(Person*, int);
void printList(struct List*);
void push(struct List**, Person*);
void deleteIndex(struct List**, int);
void initPerson(Person*);
void addPerson(struct List**);
void editPerson(struct List**);
void loadExample(struct List*);

void sort(struct List**);

void menu(struct List*);
