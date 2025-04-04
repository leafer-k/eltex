int checkExpr(char* expr){
    int dotFlag = 0, signFlag = 0, digitFlag = 0, dotDigitFlag = 0;
    int i = 0;

    if(expr[0] != '-' && isDigit(expr[0]) == 0) return 0;

    if(expr[0] == '-') {
	expr[0] = 'n';
	i = 1;
    }

    for(; expr[i] != '\0'; i++) {
	if(isSign(expr[i])) dotDigitFlag = 0;
        if((dotFlag && isDot(expr[i])) || (signFlag && isSign(expr[i]))) return 0;
	if(!(expr[0] == 'n' && i == 1)) { if(dotFlag == 0 && signFlag == 0 && digitFlag == 0 && i > 0) return 0; }
        if(isSign(expr[i]) && digitFlag == 0) return 0;
        if((isSign(expr[i]) || isDot(expr[i])) && expr[i+1] == '\0') return 0;
	if(dotDigitFlag == 1 && isDot(expr[i])) return 0;

	if(dotFlag == 1 && isDigit(expr[i])) dotDigitFlag = 1;

        dotFlag = isDot(expr[i]);
        signFlag = isSign(expr[i]);
        digitFlag = isDigit(expr[i]);

    }
    return 1;
}
