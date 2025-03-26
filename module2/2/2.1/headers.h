#define MAX_PHONE_NUMS 5
#define MAX_EMAILS 5

struct Person {
    char firstName[30];
    char lastName[30];
    char patronymic[30];
    char company[30];
    char* phones[MAX_PHONE_NUMS];
    char* emails[MAX_EMAILS];
};

void strCopy(char*, const char*, size_t);
size_t strLength(const char*);
int strCompare(const char*, const char*);
void removeNewline(char*);

void print(struct Person*, int);
void printArr(struct Person**);
void push(struct Person***, struct Person*);
void deleteIndex(struct Person***, int);
void initPerson(struct Person*);
void addPerson(struct Person***);
void editPerson(struct Person**);
void loadExample(struct Person***);

void menu(struct Person*** arr);
