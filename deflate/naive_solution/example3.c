/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

/*
	$ cat example3.txt | \
	python -c \
	'import zlib; import sys; \
	sys.stdout.write(zlib.compress(sys.stdin.read(), 9)[2:-4])'\
       	| xxd -i
*/

uint8_t deflated[] = {
	0xc5, 0x53, 0x4b, 0x6e, 0xdc, 0x30, 0x0c, 0xdd, 0xeb, 0x14, 0xf4, 0x6c,
	0xb2, 0x71, 0x7a, 0x80, 0x76, 0x51, 0x24, 0xd3, 0x45, 0x50, 0x14, 0xc8,
	0xa2, 0x73, 0x01, 0xca, 0xa6, 0x2d, 0xcd, 0x48, 0x66, 0xa0, 0x4f, 0x5c,
	0xdf, 0xbe, 0xa4, 0xec, 0x34, 0x73, 0x83, 0x6e, 0x0c, 0x81, 0xe4, 0xe3,
	0x23, 0xdf, 0xa3, 0x1f, 0x2e, 0x2b, 0x66, 0xb0, 0xc9, 0x87, 0xe0, 0xe7,
	0x1e, 0x70, 0x19, 0xa1, 0x38, 0x82, 0x1c, 0x7c, 0x71, 0x1b, 0x14, 0x7e,
	0xa7, 0x6c, 0x7e, 0xf8, 0x11, 0xe6, 0x2d, 0x51, 0xcb, 0xce, 0x3e, 0xda,
	0x40, 0xe0, 0x97, 0x56, 0xb7, 0xa2, 0xa5, 0x6f, 0xe6, 0x29, 0x04, 0x88,
	0x3e, 0xe6, 0x0d, 0x56, 0x92, 0x32, 0x4d, 0x58, 0x4e, 0x3c, 0x2b, 0xba,
	0x37, 0x4f, 0x47, 0xcf, 0xc8, 0x91, 0x20, 0x61, 0x71, 0x19, 0xb8, 0x96,
	0x39, 0x09, 0xf4, 0x8b, 0x31, 0xa7, 0x67, 0x5a, 0xf1, 0x00, 0xfd, 0x44,
	0x6b, 0x29, 0xad, 0x3c, 0xdc, 0x7a, 0x88, 0x1b, 0x64, 0x5e, 0x3a, 0x73,
	0x91, 0xf8, 0x15, 0xd7, 0x2c, 0x05, 0x58, 0xc0, 0xfa, 0x42, 0x7d, 0xab,
	0x1d, 0xc2, 0xbf, 0xe0, 0x80, 0x65, 0x70, 0x9d, 0xb9, 0x6f, 0x54, 0xed,
	0xb5, 0x5a, 0xa9, 0x4e, 0xe3, 0xbe, 0x53, 0x76, 0x75, 0x69, 0xad, 0xa6,
	0x54, 0xa3, 0xe7, 0x9a, 0xe1, 0x59, 0xc2, 0x94, 0xf2, 0xd2, 0xb0, 0x27,
	0x63, 0x5e, 0x04, 0xc9, 0x7c, 0x03, 0xe7, 0x33, 0xbc, 0x73, 0x7a, 0xc3,
	0x00, 0x79, 0xe5, 0x34, 0xea, 0xa6, 0x4e, 0x6a, 0xbf, 0x9a, 0x5f, 0xbc,
	0xcc, 0x50, 0x7c, 0xdc, 0x29, 0x22, 0x2e, 0x7f, 0x74, 0xa1, 0x89, 0x09,
	0x54, 0x30, 0xae, 0xb3, 0x2b, 0x8f, 0xe6, 0x37, 0x43, 0xa2, 0x5c, 0x68,
	0xd4, 0xa0, 0xdd, 0x5a, 0xe9, 0xa5, 0xc6, 0x52, 0x23, 0x94, 0x44, 0xb4,
	0xab, 0x91, 0x85, 0x69, 0x04, 0x5c, 0x9d, 0xff, 0x50, 0xb2, 0xa1, 0x45,
	0x0d, 0xcd, 0x8a, 0x1f, 0x12, 0xab, 0xd3, 0xe4, 0xb3, 0xfb, 0x48, 0x35,
	0x0a, 0x45, 0xf5, 0x6d, 0x8b, 0x7b, 0xa1, 0x56, 0x71, 0x0a, 0x68, 0x23,
	0x51, 0x75, 0x82, 0x29, 0x60, 0x14, 0x92, 0xb3, 0x7c, 0x41, 0xda, 0x4f,
	0x53, 0xf0, 0x3a, 0xb4, 0x4b, 0xda, 0xa5, 0x0d, 0x53, 0x6a, 0x98, 0x49,
	0x8c, 0x6a, 0xbd, 0x94, 0xce, 0xd6, 0x24, 0x86, 0xee, 0xb4, 0xaa, 0x65,
	0xa4, 0xce, 0x98, 0xd7, 0x45, 0x65, 0x5e, 0xb9, 0x83, 0xcf, 0xd7, 0x7e,
	0x1b, 0x7b, 0xa7, 0xbb, 0x77, 0x1b, 0xe8, 0x50, 0xcc, 0x06, 0x1c, 0x85,
	0x98, 0x96, 0x02, 0x79, 0xf1, 0xc3, 0x8d, 0xd2, 0xa3, 0x28, 0x3c, 0xdc,
	0x3a, 0x95, 0x37, 0xd0, 0x54, 0x94, 0x62, 0x24, 0x3c, 0x5c, 0x69, 0xa3,
	0xfb, 0x92, 0x65, 0x3b, 0x1c, 0xb5, 0xa4, 0x21, 0x67, 0x0c, 0x35, 0xbe,
	0x39, 0x1d, 0xdc, 0x0a, 0x56, 0x6f, 0x44, 0xe7, 0x74, 0x98, 0x4b, 0x93,
	0x43, 0x8e, 0x13, 0x8f, 0xf3, 0xfb, 0xd4, 0xe1, 0xbb, 0x39, 0xab, 0x1b,
	0x85, 0xf5, 0x70, 0x30, 0xc5, 0xdc, 0x2e, 0xc8, 0x12, 0x46, 0x55, 0xd1,
	0xf2, 0xd6, 0x99, 0x57, 0x31, 0x1f, 0xed, 0x55, 0xcd, 0x1f, 0x71, 0xeb,
	0xe0, 0x8c, 0x21, 0x30, 0xbb, 0xfd, 0x21, 0x81, 0x93, 0x4e, 0x30, 0x38,
	0x4e, 0x45, 0xe5, 0x50, 0xdb, 0xe5, 0x14, 0xae, 0xbc, 0xc9, 0x00, 0x0f,
	0xff, 0xf9, 0x2f, 0xf9, 0x0b
};

int readbit(uint8_t *data, int bit_index){
	int byte_index = bit_index / 8;
	int byte_bit_index = bit_index % 8;

	int bit = (data[byte_index] & (1<<byte_bit_index)) >> byte_bit_index;
	return bit;
}

#define MAX_TREE_NODES (524287)
int enc_tree[MAX_TREE_NODES]; // 2 ^^ (18+1) - 1
int lit_tree[MAX_TREE_NODES];
int dist_tree[MAX_TREE_NODES];

void build_tree(int *lengths, int nlengths, int *tree, int nnodes){
	int bl_count[16] = {0};
	int next_code[16] = {0};

	for(int i = 0; i < nlengths; i++){
		int length = lengths[i];

		if(0 <= length && length <= 15){
			bl_count[length] += 1;
		} else {
			fflush(stdout);
			fprintf(stderr, "invalid length: %d\n", length);
			exit(1);
		}
	}


	int code = 0;
	bl_count[0] = 0;
	for(int bits = 1; bits < 16; bits++){
		code = (code + bl_count[bits - 1]) << 1;
		next_code[bits] = code;
	}


	for(int i = 0; i < nnodes; i++){
		tree[i] = -1;
	}

	for(int i = 0; i < nlengths; i++){
		int l = lengths[i];
		if(l != 0){
			int code = next_code[l];
			next_code[l] += 1;

			int node_index = 0;
			for(int j = l - 1; 0 <= j; j--){
				int bit = (code >> j) & 1;
				fflush(stdout);
				assert(tree[node_index] == -1);
				if(bit == 0){
					node_index = node_index * 2 + 1;
				} else {
					node_index = node_index * 2 + 2;
				}
			}
			tree[node_index] = i;
		}
	}
}

void example3(uint8_t *data, uint8_t *out_buff, int *out_len){
	int bit_index = 0;
	int out_i = 0;

	int bfinal;
	int btype;

	bfinal = readbit(data, bit_index++);
	btype = readbit(data, bit_index++);
	btype |= readbit(data, bit_index++) << 1;

	assert(bfinal == 1);
	assert(btype == 2);


	int hlit = 0;
	int hdist = 0;
	int hclen = 0;

	hlit = 0;
	hlit |= readbit(data, bit_index++);
	hlit |= readbit(data, bit_index++) << 1;
	hlit |= readbit(data, bit_index++) << 2;
	hlit |= readbit(data, bit_index++) << 3;
	hlit |= readbit(data, bit_index++) << 4;
	hdist = 0;
	hdist |= readbit(data, bit_index++);
	hdist |= readbit(data, bit_index++) << 1;
	hdist |= readbit(data, bit_index++) << 2;
	hdist |= readbit(data, bit_index++) << 3;
	hdist |= readbit(data, bit_index++) << 4;
	hclen = 0;
	hclen |= readbit(data, bit_index++);
	hclen |= readbit(data, bit_index++) << 1;
	hclen |= readbit(data, bit_index++) << 2;
	hclen |= readbit(data, bit_index++) << 3;


	int lengths_order[19] = {
		16, 17, 18,  0,  8,  7,  9,  6,
		10,  5, 11,  4, 12,  3, 13,  2,
		14,  1, 15
	};

	int enc_lengths[19] = {0};

	for(int i = 0; i < hclen + 4; i++){
		int length = 0;
		length |= readbit(data, bit_index++);
		length |= readbit(data, bit_index++) << 1;
		length |= readbit(data, bit_index++) << 2;
		enc_lengths[lengths_order[i]] = length;
	}

	build_tree(enc_lengths, 19, enc_tree, MAX_TREE_NODES);



	int lengths[318] = {0};

	for(int i = 0; i < hlit + 257 + hdist + 1; i++){
		int n = 0;
		while(enc_tree[n] == -1){
			if(readbit(data, bit_index++) == 0){
				n = n * 2 + 1;
			} else {
				n = n * 2 + 2;
			}
		}
		int symbol = enc_tree[n];

		if(0 <= symbol && symbol <= 15){
			lengths[i] = symbol;
		} else if(symbol == 16){
			if(0 < i){
				int rep = 0;
				rep |= readbit(data, bit_index++);
				rep |= readbit(data, bit_index++) << 1;
				rep += 3;
				int last = lengths[i-1];
				for(int j = 0; j < rep; j++){
					lengths[i+j] = last;
				}
				i += rep - 1;
			} else {
				fprintf(stderr, "Can't repeat last code without a last code!\n");
				exit(1);
			}
		} else if(symbol == 17){
			int count = 0;
			count |= readbit(data, bit_index++);
			count |= readbit(data, bit_index++) << 1;
			count |= readbit(data, bit_index++) << 2;
			count += 3;

			for(int j = 0; j < count; j++){
				lengths[i+j] = 0;
			}
			i += count - 1;
		} else if(symbol == 18){
			int count = 0;
			count |= readbit(data, bit_index++);
			count |= readbit(data, bit_index++) << 1;
			count |= readbit(data, bit_index++) << 2;
			count |= readbit(data, bit_index++) << 3;
			count |= readbit(data, bit_index++) << 4;
			count |= readbit(data, bit_index++) << 5;
			count |= readbit(data, bit_index++) << 6;
			count += 11;

			for(int j = 0; j < count; j++){
				lengths[i+j] = 0;
			}
			i += count - 1;
		}
	}

	build_tree(lengths, hlit+257, lit_tree, MAX_TREE_NODES);
	build_tree(lengths + hlit + 257, hdist+1,  dist_tree, MAX_TREE_NODES);

	for(;;){
		int n = 0;
		while(lit_tree[n] == -1){
			if(readbit(data, bit_index++) == 0){
				n = n * 2 + 1;
			} else {
				n = n * 2 + 2;
			}
		}

		int lit_symbol = lit_tree[n];

		if(0 <= lit_symbol && lit_symbol <= 255){
			out_buff[out_i++] = lit_symbol;
		} else if(lit_symbol == 256){
			break;
		} else if(257 <= lit_symbol && lit_symbol <= 285){
			int length;

			int nextra;
			int extra;

			int lit_extra_lengths[] = {
				0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
				1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
				4, 4, 4, 4, 5, 5, 5, 5, 0
			};
			nextra = lit_extra_lengths[lit_symbol - 257];

			extra = 0;
			for(int i = 0; i < nextra; i++){
				extra |= readbit(data, bit_index++) << i;
			}

			int lit_lengths[] = {
				3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
				15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
				67, 83, 99, 115, 131, 163, 195, 227, 258
			};

			length = lit_lengths[lit_symbol - 257] + extra;

			n = 0;
			while(dist_tree[n] == -1){
				if(readbit(data, bit_index++) == 0){
					n = n * 2 + 1;
				} else {
					n = n * 2 + 2;
				}
			}
			int dist_symbol = dist_tree[n];

			if(0 <= dist_symbol && dist_symbol <= 29){
				int dist_extra_list[] = {
					0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
					4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
					9, 9, 10, 10, 11, 11, 12, 12, 13, 13
				};

				nextra = dist_extra_list[dist_symbol];

				extra = 0;
				for(int i = 0; i < nextra; i++){
					extra |= readbit(data, bit_index++) << i;
				}

				int distances[] = {
					1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
					33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
					1024, 1537, 2049, 3073, 4097, 6145,
					8193, 12289, 16385, 24577
				};

				int distance = distances[dist_symbol] + extra;

				for(int i = 0; i < length; i++){
					out_buff[out_i] = out_buff[out_i - distance];
					out_i += 1;
				}
			} else {
				fflush(stdout);
				fprintf(stderr, "invalid distance symbol: %d\n", dist_symbol);
				exit(1);
			}
		} else {
			fflush(stdout);
			fprintf(stderr, "invalid lit/len symbol: %d\n", lit_symbol);
			exit(1);
		}
	}

	*out_len = out_i;
}

int main(int argc, char *argv[]){
	char buff[1024];
	int len;

	example3(deflated, (uint8_t *)buff, &len);

	printf("Length: %d\n", len);
	printf("Data: %*s\n", len, buff);

	return 0;
}
