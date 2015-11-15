/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>

void example1(){
	// ('z') ('z') (18, 3)
	char buff[1024];
	int i;

	i = 0;

	buff[i++] = 'z';
	buff[i++] = 'z';

	for(int j = 0; j < 18; j++){
		buff[i] = buff[i - 2];
		i += 1;
	}

	buff[i] = 0;

	printf("output: %s\n", buff);
}

void example2(){
	char buff[1024];
	int i;

	i = 0;
	buff[i++] = 'a';
	buff[i++] = 'b';
	buff[i++] = 'a';

	for(int j = 0; j < 17; j++){
		buff[i] = buff[i - 1];
		i += 1;
	}

	buff[i] = 0;

	printf("output: %s\n", buff);
}


int main(int argc, char *argv[]){
	example1();
	example2();
	return 0;
}
