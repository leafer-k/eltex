int isDigit(char c) {
    for(int j = 0; j < 10; j++){
	if(c == '0' + j) {
	    return 1;
	}
    }
    return 0;
}
