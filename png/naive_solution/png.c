/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <zlib.h>


int readfile(char *filename, uint8_t **filedata, size_t *filesize){
	FILE *f;
	size_t size;
	uint8_t *buff;

	f = fopen(filename, "r");
	if(f){
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);

		buff = malloc(size);
		if(buff){
			if(fread(buff, size, 1, f) == 1){
				fclose(f);
				*filedata = buff;
				*filesize = size;
				return 0;
			} else {
				fprintf(stderr, "error reading file\n");
				exit(1);
			}
		} else {
			fprintf(stderr, "error on malloc()\n");
			exit(1);
		}
	} else {
		fprintf(stderr, "error opening file\n");
		exit(1);
	}
}

struct png_header {
	uint32_t width;
	uint32_t height;
	uint8_t bitDepth;
	uint8_t colorType;
	uint8_t compressionMethod;
	uint8_t filterMethod;
	uint8_t interlaceMethod;
};

void dopng(uint8_t *data){
	uint8_t magic[] = {137, 80, 78, 71, 13, 10, 26, 10};

	if(memcmp(data, magic, sizeof(magic)) != 0){
		fprintf(stderr, "invalid file magic");
		exit(1);
	}

	int byte_index = 8;

	uint32_t length;
	uint32_t type;

	length = data[byte_index++] << 24;
	length |= data[byte_index++] << 16;
	length |= data[byte_index++] << 8;
	length |= data[byte_index++];

	type = data[byte_index++] << 24;
	type |= data[byte_index++] << 16;
	type |= data[byte_index++] << 8;
	type |= data[byte_index++];

	if(type != 0x49484452){
		fprintf(stderr, "first chunk must be IHDR: %08x\n", type);
		exit(1);
	}

	if(length != 13){
		fprintf(stderr, "unknown IHDR format\n");
		exit(1);
	}

	struct png_header header;
	header.width = data[byte_index++] << 24;
	header.width |= data[byte_index++] << 16;
	header.width |= data[byte_index++] << 8;
	header.width |= data[byte_index++];
	header.height = data[byte_index++] << 24;
	header.height |= data[byte_index++] << 16;
	header.height |= data[byte_index++] << 8;
	header.height |= data[byte_index++];
	header.bitDepth = data[byte_index++];
	header.colorType = data[byte_index++];
	header.compressionMethod = data[byte_index++];
	header.filterMethod = data[byte_index++];
	header.interlaceMethod = data[byte_index++];

	assert(header.height == 16);
	assert(header.width == 16);
	assert(header.bitDepth == 8);
	assert(header.colorType == 6);
	assert(header.compressionMethod == 0);
	assert(header.filterMethod == 0);
	assert(header.interlaceMethod == 0);

	uint32_t crc;
	crc = data[byte_index++] << 24;
	crc |= data[byte_index++] << 16;
	crc |= data[byte_index++] << 8;
	crc |= data[byte_index++];

	assert(crc == 0x1ff3ff61);

	length = data[byte_index++] << 24;
	length |= data[byte_index++] << 16;
	length |= data[byte_index++] << 8;
	length |= data[byte_index++];

	type = data[byte_index++] << 24;
	type |= data[byte_index++] << 16;
	type |= data[byte_index++] << 8;
	type |= data[byte_index++];

	byte_index += length;
	byte_index += 4;

	length = data[byte_index++] << 24;
	length |= data[byte_index++] << 16;
	length |= data[byte_index++] << 8;
	length |= data[byte_index++];

	type = data[byte_index++] << 24;
	type |= data[byte_index++] << 16;
	type |= data[byte_index++] << 8;
	type |= data[byte_index++];

	assert(type == 0x49444154);

	crc = data[byte_index+length] << 24;
	crc |= data[byte_index+length+1] << 16;
	crc |= data[byte_index+length+2] << 8;
	crc |= data[byte_index+length+3];


	uLong calculatedCRC = crc32(0, Z_NULL, 0);
	calculatedCRC = crc32(calculatedCRC, &data[byte_index-4], length + 4);

	assert(crc == calculatedCRC);


	uint8_t outdata[1040];
	z_stream zstream;

	zstream.next_in = &data[byte_index];
	zstream.avail_in = length;
	zstream.next_out = outdata;
	zstream.avail_out = 1040;
	zstream.zalloc = Z_NULL;
	zstream.zfree = Z_NULL;
	zstream.opaque = Z_NULL;

	if(inflateInit(&zstream) != Z_OK){
		fprintf(stderr, "zlib error on inflateInit()\n");
		exit(1);
	}

	int err;

	err = inflate(&zstream, Z_NO_FLUSH);
	if(err != Z_STREAM_END){
		fprintf(stderr, "zlib error on inflate(): %d\n", err);
		exit(1);
	}

	err = inflateEnd(&zstream);
	if(err != Z_OK){
		fprintf(stderr, "zlib error on inflateEnd(): %d\n", err);
		exit(1);
	}

	for(int row = 0; row < 16; row++){
		for(int col = 0; col < 16; col++){
			uint32_t rgba;
			rgba = outdata[row * 65 + 1 + col * 4 + 0] << 24;
			rgba |= outdata[row * 65 + 1 + col * 4 + 1] << 16;
			rgba |= outdata[row * 65 + 1 + col * 4 + 2] << 8;
			rgba |= outdata[row * 65 + 1 + col * 4 + 3];

			if(rgba == 0x000000){
				printf(" ");
			} else if(rgba == 0x000000ff){
				printf("#");
			} else if(rgba == 0xffffffff){
				printf(".");
			}
		}
		printf("\n");
	}
}


int main(int argc, char *argv[]){
	uint8_t *filedata;
	size_t filesize;

	readfile("cursor1.png", &filedata, &filesize);
	printf("filesize: %lu\n", filesize);
	dopng(filedata);
	return 0;
}
