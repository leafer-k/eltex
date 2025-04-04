void clearSpaces(char* str) {
    int spaces = 0, start = 0;
    for(int i = 0; str[i] != '\0'; i++) {
	if(str[i] == ' ') {
	    spaces++;
	    if (start == 0) start = i;
	}
    }

    for(int i = start; ; i++) {
	str[i] = str[i+spaces];
	if(str[i] == '\0') break;
    }
}
