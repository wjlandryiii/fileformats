/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

uint8_t deflated[] = {
	0x01, 0x0c, 0x00, 0xf3, 0xff, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57,
	0x6f, 0x72, 0x6c, 0x64, 0x21
};


void example1(uint8_t *data, uint8_t *out_buff, int *out_len){
	int bfinal;
	int btype;

	bfinal = data[0] & 1;
	btype = (data[0] >> 1) & 3;

	printf("BFINAL: %d\n", bfinal);
	printf("BTYPE: %d\n", btype);

	assert(bfinal == 1);
	assert(btype == 0);

	int len;
	int nlen;

	len = data[1] | (data[2] << 8);
	nlen = data[3] | (data[4] << 8);

	printf("LEN: %d\n", len);

	assert(len == (~nlen & 0xffff));

	memcpy(out_buff, &data[5], len);
	*out_len = len;
}


int main(int argc, char *argv[]){
	char buff[1024];
	int len;

	example1(deflated, (uint8_t *)buff, &len);

	printf("Data: %*s\n", len, buff);

	return 0;
}
