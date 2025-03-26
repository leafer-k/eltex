float sum(float, float);
float substract(float, float);
float multi(float, float);
float division(float, float);

float (*getOperation(char))(float, float);

int isDigit(char);
int isDot(char);
int isSign(char);

int checkExpr(char*);

float simpleSolve(char*);

void clearSpaces(char*);

float solve(char*);
