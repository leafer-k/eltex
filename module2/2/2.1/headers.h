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

void strCopy(char*, const char*, size_t);
size_t strLength(const char*);
int strCompare(const char*, const char*);
void removeNewline(char*);

void print( Person*, int);
void printArr( Person**);
void push( Person***,  Person*);
void deleteIndex( Person***, int);
void initPerson( Person*);
void addPerson( Person***);
void editPerson( Person**);
void loadExample( Person***);

void menu( Person*** arr);
