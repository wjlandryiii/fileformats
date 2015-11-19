/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

static short lit_tree_fixed[1024];                  /* max_bits:9 */
static short dist_tree_fixed[64];                   /* max_bits:5 */
static short enc_tree[512];                         /* max_bits:8 */
static short lit_tree_dynamic[65536];               /* max_bits:15 */
static short dist_tree_dynamic[65536];              /* max_bits:15 */

#define ARRAY_COUNT(A) (sizeof(A)/sizeof(A[0]))

static void build_fixed_trees(void){
	for(int i = 0; i < ARRAY_COUNT(lit_tree_fixed); i++){
		lit_tree_fixed[i] = -1;
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
			assert(lit_tree_fixed[n] == -1);
			if((code & (1<<j)) == 0){
				n = n * 2 + 1;
			} else {
				n = n * 2 + 2;
			}
		}
		assert(lit_tree_fixed[n] == -1);
		lit_tree_fixed[n] = i;
	}

	for(int i = 0; i < ARRAY_COUNT(dist_tree_fixed); i++){
		dist_tree_fixed[i] = -1;
	}

	for(int i = 0; i < 32; i++){
		int n = 0;
		for(int j = 4; 0 <= j; j--){
			assert(dist_tree_fixed[n] == -1);
			if((i & (1<<j)) == 0){
				n = n * 2 + 1;
			} else {
				n = n * 2 + 2;
			}
		}
		assert(dist_tree_fixed[n] == -1);
		dist_tree_fixed[n] = i;
	}
}

static void build_tree(int *lengths, int nlengths, short *tree, int nnodes){
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
				if(node_index >= nnodes){
					printf("n: %d\n", node_index);
					printf("nnodes: %d\n", nnodes);
				}
				assert(node_index < nnodes);
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

static int readbit(uint8_t *data, int bit_index){
	int byte_index = bit_index / 8;
	int byte_bit_index = bit_index % 8;

	int bit = (data[byte_index] & (1<<byte_bit_index)) >> byte_bit_index;
	return bit;
}

static int readint(uint8_t *data, int *bit_index, int nbits){
	int result;

	result = 0;
	for(int i = 0; i < nbits; i++){
		result |= readbit(data, *bit_index) << i;
		*bit_index += 1;
	}
	return result;
}

static int readsymbol(uint8_t *data, int *bit_index, short *tree){
	int n = 0;

	while(tree[n] == -1){
		if(readbit(data, *bit_index) == 0){
			n = n * 2 + 1;
		} else {
			n = n * 2 + 2;
		}
		*bit_index += 1;
	}
	return tree[n];
}

int inflate(uint8_t *input, int input_len, int *input_used, uint8_t *output, int output_size, int *output_len){
	static int initialized = 0;

	if(!initialized){
		build_fixed_trees();
	}
	int bit_index = 0;
	int out_index = 0;
	int bfinal;
	int btype;

	do {
		bfinal = readbit(input, bit_index++);
		btype = readint(input, &bit_index, 2);

		if(btype == 0){
			if(bit_index % 8 != 0){
				bit_index += 8 - (bit_index % 8);
			}

			int len, nlen;
			len = input[bit_index/8] | input[(bit_index/8)+1] << 8;
			nlen = input[(bit_index/8)+2] | input[(bit_index/8)+3] << 8;
			bit_index += 32;

			if(len != (~nlen & 0xffff)){
				fflush(stdout);
				fprintf(stderr, "invalid len/nlen pair\n");
				exit(1);
			}

			for(int i = 0; i < len; i++){
				output[out_index++] = input[bit_index/8];
				bit_index += 8;
			}
		} else if(btype == 1 || btype == 2){
			short *lit_tree;
			short *dist_tree;
			if(btype == 1){
				lit_tree = lit_tree_fixed;
				dist_tree = dist_tree_fixed;
			} else {
				int hlit, hdist, hclen;

				hlit = readint(input, &bit_index, 5);
				hdist = readint(input, &bit_index, 5);
				hclen = readint(input, &bit_index, 4);

				static int order[19] = {
					16, 17, 18,  0,  8,  7,  9,  6,
					10,  5, 11,  4, 12,  3, 13,  2,
					14,  1, 15
				};

				int enc_lengths[19] = {0};

				for(int i = 0; i < hclen + 4; i++){
					enc_lengths[order[i]] = readint(input, &bit_index, 3);
				}
				build_tree(enc_lengths, 19, enc_tree, ARRAY_COUNT(enc_tree));
				int lengths[318] = {0};

				for(int i = 0; i < hlit + 257 + hdist + 1; i++){
					int symbol = readsymbol(input, &bit_index, enc_tree);

					if(0 <= symbol && symbol <= 15){
						lengths[i] = symbol;
					} else if(symbol == 16){
						if(0 < i){
							int rep = readint(input, &bit_index, 2);
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
						count = readint(input, &bit_index, 3);
						count += 3;

						for(int j = 0; j < count; j++){
							lengths[i+j] = 0;
						}
						i += count - 1;
					} else if(symbol == 18){
						int count = 0;
						count = readint(input, &bit_index, 7);
						count += 11;

						for(int j = 0; j < count; j++){
							lengths[i+j] = 0;
						}
						i += count - 1;
					}
				}

				build_tree(lengths, hlit + 257, lit_tree_dynamic, ARRAY_COUNT(lit_tree_dynamic));
				build_tree(lengths + hlit + 257, hdist + 1,  dist_tree_dynamic, ARRAY_COUNT(dist_tree_dynamic));

				lit_tree = lit_tree_dynamic;
				dist_tree = dist_tree_dynamic;
			}

			for(;;){
				int lit_symbol = readsymbol(input, &bit_index, lit_tree);

				if(0 <= lit_symbol && lit_symbol <= 255){
					output[out_index++] = lit_symbol;
				} else if(lit_symbol == 256){
					break;
				} else if(257 <= lit_symbol && lit_symbol <= 285){

					int nextra, extra;
#if 0
					static int lit_extra_lengths[] = {
						0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
						1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
						4, 4, 4, 4, 5, 5, 5, 5, 0
					};
					nextra = lit_extra_lengths[lit_symbol - 257];
#else
					nextra = lit_symbol - 257 < 28 ? ((lit_symbol - 257) >> 2) - 1 : 0;
#endif
					extra = readint(input, &bit_index, nextra);

					static int lit_lengths[] = {
						3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
						15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
						67, 83, 99, 115, 131, 163, 195, 227, 258
					};

					int length = lit_lengths[lit_symbol - 257] + extra;

					int dist_symbol = readsymbol(input, &bit_index, dist_tree);

					if(0 <= dist_symbol && dist_symbol <= 29){
						static int dist_extra_list[] = {
							0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
							4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
							9, 9, 10, 10, 11, 11, 12, 12, 13, 13
						};
						nextra = dist_extra_list[dist_symbol];
						extra = readint(input, &bit_index, nextra);
						static int distances[] = {
							1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
							33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
							1024, 1537, 2049, 3073, 4097, 6145,
							8193, 12289, 16385, 24577
						};

						int distance = distances[dist_symbol] + extra;

						for(int i = 0; i < length; i++){
							output[out_index] = output[out_index - distance];
							out_index += 1;
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
		} else {
			fflush(stdout);
			fprintf(stderr, "invalid btype: %d\n", btype);
			exit(1);
		}
	} while(bfinal == 0);

	*input_used = bit_index / 8 + (bit_index % 8 != 0);
	*output_len = out_index;
	return 0;
}
