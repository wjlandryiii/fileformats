/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

// cat example2.txt | \
// python -c \
// 'import zlib; import sys; \
// sys.stdout.write(zlib.compress(sys.stdin.read(), 9)[2:-4])' | \
// xxd -i -

uint8_t deflated[] = {
	0x73, 0xce, 0x28, 0xca, 0x2c, 0x2e, 0xc9, 0xcc, 0x4b, 0xd5, 0x51, 0x28,
	0xc9, 0x48, 0x55, 0x28, 0x2e, 0x29, 0x4a, 0x2c, 0x4f, 0x4a, 0x2d, 0x2a,
	0xaa, 0x54, 0x48, 0xcf, 0x2c, 0xca, 0xe1, 0x72, 0x46, 0x48, 0x27, 0x25,
	0xe6, 0x01, 0xa1, 0x42, 0x71, 0x41, 0x4e, 0x66, 0x89, 0x42, 0x4e, 0x62,
	0x4a, 0x25, 0xb2, 0x24, 0x5e, 0xbd, 0x0a, 0xc5, 0xa9, 0xa9, 0xc5, 0x0a,
	0x19, 0xa9, 0x45, 0x0a, 0x69, 0x89, 0xc9, 0x40, 0x56, 0x69, 0x5e, 0x5a,
	0x29, 0x50, 0x1e, 0x00
};

int lit_tree[1024];
int dist_tree[64];

void build_tree(void){
	for(int i = 0; i < sizeof(lit_tree)/sizeof(lit_tree[0]); i++){
		lit_tree[i] = -1;
	}

	for(int i = 0; i <= 287; i++){
		int len;
		int code;

		if(0 <= i && i <= 143){
			len = 8;
			code = i + 48;
		} else if(144 <= i && i <= 255){
			len = 9;
			code = i + 256;
		} else if(256 <= i && i <= 279){
			len = 7;
			code = i - 256;
		} else if(280 <= i && i <= 287){
			len = 8;
			code = i - 88;
		}

		int n = 0;
		for(int j = len - 1; 0 <= j; j--){
			assert(lit_tree[n] == -1);
			if((code & (1<<j)) == 0){
				n = n * 2 + 1;
			} else {
				n = n * 2 + 2;
			}
		}
		assert(lit_tree[n] == -1);
		lit_tree[n] = i;
	}

	for(int i = 0; i < sizeof(dist_tree)/sizeof(dist_tree[0]); i++){
		dist_tree[i] = -1;
	}

	for(int i = 0; i < 32; i++){
		int n = 0;
		for(int j = 4; 0 <= j; j--){
			assert(dist_tree[n] == -1);
			if((i & (1<<j)) == 0){
				n = n * 2 + 1;
			} else {
				n = n * 2 + 2;
			}
		}
		assert(dist_tree[n] == -1);
		dist_tree[n] = i;
	}
}

int readbit(uint8_t *data, int bit_index){
	int byte_index = bit_index / 8;
	int byte_bit_index = bit_index % 8;

	int bit = (data[byte_index] & (1<<byte_bit_index)) >> byte_bit_index;
	return bit;
}

void example2(uint8_t *data, uint8_t *out_buff, int *out_len){
	int bit_index = 0;
	int bfinal;
	int btype;
	int out_i = 0;

	bfinal = readbit(data, bit_index++);
	btype = readbit(data, bit_index++);
	btype |= readbit(data, bit_index++) << 1;

	assert(bfinal == 1);
	assert(btype == 1);

	for(;;){
		int symbol;
		int length;
		int distance;
		int n;

		n = 0;
		for(int i = 0; i < 18; i++){
			int bit = readbit(data, bit_index++);
			if(bit == 0){
				n = n * 2 + 1;
			} else {
				n = n * 2 + 2;
			}

			if(lit_tree[n] != -1){
				break;
			}
		}

		symbol = lit_tree[n];

		if(0 <= symbol && symbol <= 255){
			out_buff[out_i++] = symbol;
		} else if(symbol == 256){
			break;
		} else if(257 <= symbol && symbol <= 285){
			int extra;
			int nextra;

			int lit_extra_lengths[] = {
				0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
				1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
				4, 4, 4, 4, 5, 5, 5, 5, 0
			};

			nextra = lit_extra_lengths[symbol - 257];

			extra = 0;
			for(int i = 0; i < nextra; i++){
				extra |= readbit(data, bit_index++) << i;
			}

			int lengths[] = {
				3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
				15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
				67, 83, 99, 115, 131, 163, 195, 227, 258
			};

			length = lengths[symbol - 257] + extra;

			n = 0;
			while(dist_tree[n] == -1){
				if(readbit(data, bit_index++) == 0){
					n = n * 2 + 1;
				} else {
					n = n * 2 + 2;
				}
			}
			symbol = dist_tree[n];

			if(0 <= symbol && symbol <= 29){
				int dist_extra_list[] = {
					0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
					4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
					9, 9, 10, 10, 11, 11, 12, 12, 13, 13
				};
				nextra = dist_extra_list[symbol];

				extra = 0;
				for(int i = 0; i < nextra; i++){
					extra |= readbit(data, bit_index++) << i;
				}

				int distances[] = {
					1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
					33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
					1024, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
				};

				distance = distances[symbol] + extra;

				for(int i = 0; i < length; i++){
					out_buff[out_i] = out_buff[out_i - distance];
					out_i += 1;
				}
			} else {
				fflush(stdout);
				fprintf(stderr, "invalid distance symbol: %d\n", symbol);
				exit(1);
			}

		} else {
			fflush(stdout);
			fprintf(stderr, "Invalid symbol: %d\n", symbol);
			exit(1);
		}
	}
	*out_len = out_i;
}

int main(int argc, char *argv[]){
	char buff[1024];
	int len;

	build_tree();
	example2(deflated, (uint8_t *)buff, &len);

	printf("Length: %d\n", len);
	printf("Data: %*s\n", len, buff);

	return 0;
}

