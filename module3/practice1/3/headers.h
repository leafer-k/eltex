#define MAX_PHONE_NUMS 5
#define MAX_EMAILS 5
#define MAX_PHONE_LEN 15
#define MAX_EMAIL_LEN 75
#define MAX_NAME_LEN 30
#define MAX_LNAME_LEN 30
#define MAX_PAT_LEN 30
#define MAX_COMPANY_LEN 30


typedef struct {
    char firstName[MAX_NAME_LEN];
    char lastName[MAX_LNAME_LEN];
    char patronymic[MAX_PAT_LEN];
    char company[MAX_COMPANY_LEN];
    char phones[MAX_PHONE_NUMS][MAX_PHONE_LEN];
    char emails[MAX_EMAILS][MAX_EMAIL_LEN];
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

int loadFromFile(char*, struct List**);
int saveToFile(char*, struct List**);

void menu(struct List*);
