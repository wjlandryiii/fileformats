/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

uint8_t *deflated = (uint8_t *) "\xab\xaa\xc2\x04\x00";

struct tree_node {
	int is_leaf;
	int symbol;
};

struct tree_node length_tree_fixed[524288]; // 2 ^^ 19
/*
struct tree_node fixed_distance_tree[2^19];
struct tree_node dynamic_length_value_tree[2^19];
struct tree_node dynamic_distance_tree[2^19];
*/

char *tobin(uint32_t value, int bits){
	static int n = 0;
	static char buff[4][33];
	char *p;

	if(n == 4){
		n = 0;
	}

	p = buff[n];
	for(int i = 31; 0 <= i; i--){
		*p++ = ((value >> i) & 1) == 0 ? '0' : '1';
	}
	*p = 0;
	return &buff[n++][32-bits];
}

int readbit(uint8_t *data, int bit_index){
	int byte_index = bit_index / 8;
	int byte_bit_index = bit_index % 8;

	int bit = (data[byte_index] & (1<<byte_bit_index)) >> byte_bit_index;
	return bit;
}


/*
	RFC1951 page 12:

                 Extra               Extra               Extra
            Code Bits Length(s) Code Bits Lengths   Code Bits Length(s)
            ---- ---- ------     ---- ---- -------   ---- ---- -------
             257   0     3       267   1   15,16     277   4   67-82
             258   0     4       268   1   17,18     278   4   83-98
             259   0     5       269   2   19-22     279   4   99-114
             260   0     6       270   2   23-26     280   4  115-130
             261   0     7       271   2   27-30     281   5  131-162
             262   0     8       272   2   31-34     282   5  163-194
             263   0     9       273   3   35-42     283   5  195-226
             264   0    10       274   3   43-50     284   5  227-257
             265   1  11,12      275   3   51-58     285   0    258
             266   1  13,14      276   3   59-66
*/

int length_extra_bits[][2] = {
/* Code: 257 */ {0, 3},
/* Code: 258 */ {0, 4},
/* Code: 259 */ {0, 5},
/* Code: 260 */ {0, 6},
/* Code: 261 */ {0, 7},
/* Code: 262 */ {0, 8},
/* Code: 263 */ {0, 9},
/* Code: 264 */ {0, 10},
/* Code: 265 */ {1, 11},
/* Code: 266 */ {1, 13},
/* Code: 267 */ {1, 15},
/* Code: 268 */ {1, 17},
/* Code: 269 */ {2, 19},
/* Code: 270 */ {2, 23},
/* Code: 271 */ {2, 27},
/* Code: 272 */ {2, 31},
/* Code: 273 */ {3, 35},
/* Code: 274 */ {3, 43},
/* Code: 275 */ {3, 51},
/* Code: 276 */ {3, 59},
/* Code: 277 */ {4, 67},
/* Code: 278 */ {4, 83},
/* Code: 279 */ {4, 99},
/* Code: 280 */ {4, 115},
/* Code: 281 */ {5, 131},
/* Code: 282 */ {5, 163},
/* Code: 283 */ {5, 195},
/* Code: 284 */ {5, 227},
/* Code: 285 */ {0, 258},
};

int length_symbol_has_extra_bits(int symbol){
	if(0 <= symbol && symbol <= 256){
		return 0;
	} else if(257 <= symbol && symbol <= 285){
		return 1;
	} else if(286 <= symbol && symbol <= 287){
		return 0;
	} else {
		fflush(stdout);
		fprintf(stderr, "invalid length symbol: %d line:%d", symbol, __LINE__);
		exit(1);
	}

}
int extra_bits_count_for_length_symbol(int symbol){
	if(0 <= symbol && symbol <= 256){
		return 0;
	} else if(257 <= symbol && symbol <= 285){
		return length_extra_bits[symbol - 257][0];
	} else if(286 <= symbol && symbol <= 287){
		return 0;
	} else {
		fflush(stdout);
		fprintf(stderr, "invalid length symbol: %d", symbol);
		exit(1);
	}
}
int length_from_length_symbol_and_extra_bits(int value, int extra_bits){
	if(256 <= value && value <= 285){
		return length_extra_bits[value - 257][1] + extra_bits;
	} else {
		fflush(stdout);
		fprintf(stderr, "invalid length value: %d\n", value);
		exit(1);
	}
}

/*
	RFC1951 page 12:

                  Extra           Extra               Extra
             Code Bits Dist  Code Bits   Dist     Code Bits Distance
             ---- ---- ----  ---- ----  ------    ---- ---- --------
               0   0    1     10   4     33-48    20    9   1025-1536
               1   0    2     11   4     49-64    21    9   1537-2048
               2   0    3     12   5     65-96    22   10   2049-3072
               3   0    4     13   5     97-128   23   10   3073-4096
               4   1   5,6    14   6    129-192   24   11   4097-6144
               5   1   7,8    15   6    193-256   25   11   6145-8192
               6   2   9-12   16   7    257-384   26   12  8193-12288
               7   2  13-16   17   7    385-512   27   12 12289-16384
               8   3  17-24   18   8    513-768   28   13 16385-24576
               9   3  25-32   19   8   769-1024   29   13 24577-32768

*/

int distance_extra_bits[][2] = {
/* code: 0 */ {0, 1},
/* code: 1 */ {0, 2},
/* code: 2 */ {0, 3},
/* code: 3 */ {0, 4},
/* code: 4 */ {1, 5},
/* code: 5 */ {1, 7},
/* code: 6 */ {2, 9},
/* code: 7 */ {2, 13},
/* code: 8 */ {3, 17},
/* code: 9 */ {3, 25},
/* code:10 */ {4, 33},
/* code:11 */ {4, 49},
/* code:12 */ {5, 65},
/* code:13 */ {5, 97},
/* code:14 */ {6, 129},
/* code:15 */ {6, 193},
/* code:16 */ {7, 257},
/* code:17 */ {7, 385},
/* code:18 */ {8, 513},
/* code:19 */ {8, 769},
/* code:20 */ {9, 1025},
/* code:21 */ {9, 1537},
/* code:22 */ {10, 2049},
/* code:23 */ {10, 3073},
/* code:24 */ {11, 4097},
/* code:25 */ {11, 6145},
/* code:26 */ {12, 8193},
/* code:27 */ {12, 12289},
/* code:28 */ {13, 16385},
/* code:29 */ {13, 24577},
};

int extra_bits_for_distance_code(int code){
	if(0 <= code && code <= 29){
		return distance_extra_bits[code][0];
	} else {
		fflush(stdout);
		fprintf(stderr, "invalid distance code\n");
		exit(1);
	}
}

/*
	RFC1951 page 12:
                   Lit Value    Bits        Codes
                   ---------    ----        -----
                     0 - 143     8          00110000 through
                                            10111111
                   144 - 255     9          110010000 through
                                            111111111
                   256 - 279     7          0000000 through
                                            0010111
                   280 - 287     8          11000000 through
                                            11000111
*/

int length_value_bits[4][4] = {
	{0, 143, 8, 48},
	{144, 255, 9, 256},
	{256, 279, 7, -256},
	{280, 297, 8, -88},
};

int fixed_length_literal_code_from_symbol(int symbol){
	for(int i = 0; i < 4; i++){
		if(length_value_bits[i][0] <= symbol && symbol <= length_value_bits[i][1]){
			return symbol + length_value_bits[i][3];
		}
	}
	fflush(stdout);
	fprintf(stderr, "invalid length value (likely programmer error)\n");
	exit(1);
}

int is_value_literal(int value){
	if(0 <= value && value <= 255){
		return 1;
	} else if(256 <= value && value <= 287){
		return 0;
	} else {
		fflush(stdout);
		fprintf(stderr, "invalid value (likely programmer error)\n");
		exit(1);
	}
}

int fixed_length_literal_code_length_from_symbol(int symbol){
	for(int i = 0; i < 4; i++){
		if(length_value_bits[i][0] <= symbol && symbol <= length_value_bits[i][1]){
			return length_value_bits[i][2];
		}
	}
	fflush(stdout);
	fprintf(stderr, "invalid length value (likely programmer error)\n");
	exit(1);
}


/*
 * Build tree described in section 3.2.6.
 */
void build_fixed_trees(){
	int code;
	int code_length;
	int code_extra_bits;
	int i, j, k;
	int node_index;

	int literal_value;
	int length_value;

	int full_code;
	int full_code_length;


	memset(length_tree_fixed, 0, sizeof(length_tree_fixed));

	for(i = 0; i < 288; i++){
		int code = fixed_length_literal_code_from_symbol(i);
		int code_length = fixed_length_literal_code_length_from_symbol(i);

		int node_index = 0;
		for(j = code_length - 1; 0 <= j; j--){
			int bit = (code & (1<<j)) >> j;
			if(bit == 0){
				node_index = node_index * 2 + 1;
			} else {
				node_index = node_index * 2 + 2;
			}
			assert(length_tree_fixed[node_index].is_leaf == 0);
		}
		length_tree_fixed[node_index].is_leaf = 1;
		length_tree_fixed[node_index].symbol = i;
	}
}

void example1(){
	uint8_t *data = (uint8_t *)"\xab\xaa\xc2\x04\x00";
	int bit_index = 0;
	int bfinal, btype;
	int bit;
	int node_index;
	int length_code;
	int nbits;
	int extra_bits_count;
	int extra_bits;
	int symbol;
	int length;
	int distance_code;
	int distance_extra_bits_count;
	int distance_extra_bits;
	int distance;

	// READ BLOCK HEADER
	bfinal = readbit(data, bit_index++);
	btype = 0;
	btype |= readbit(data, bit_index++) << 0;
	btype |= readbit(data, bit_index++) << 1;
	printf("BFINAL: %s\n", tobin(bfinal, 1));
	printf("BTYPE : %s\n", tobin(btype, 2));
	printf("\n");

	assert(bfinal == 1);
	assert(btype == 1);

	// read first length code;
	node_index = 0;
	length_code = 0;
	for(int i = 0; i < 18; i++){
		bit = readbit(data, bit_index++);
		length_code = (length_code << 1) | bit;
		nbits = i + 1;
		if(bit == 0){
			node_index = node_index * 2 + 1;
		} else {
			node_index = node_index * 2 + 2;
		}
		if(length_tree_fixed[node_index].is_leaf){
			break;
		}
	}

	printf("LENGHT_CODE_LENGHT: %d\n", nbits);
	printf("LENGTH_CODE: %s\n", tobin(length_code, nbits));
	printf("LENGTH_VALUE: %d\n", length_tree_fixed[node_index].symbol);
	printf("\n");

	node_index = 0;
	length_code = 0;
	for(int i = 0; i < 18; i++){
		bit = readbit(data, bit_index++);
		length_code = (length_code << 1) | bit;
		nbits = i + 1;
		if(bit == 0){
			node_index = node_index * 2 + 1;
		} else {
			node_index = node_index * 2 + 2;
		}
		if(length_tree_fixed[node_index].is_leaf){
			break;
		}
	}
	printf("LENGTH_CODE_LENGTH: %d\n", nbits);
	printf("LENGTH_CODE: %s\n", tobin(length_code, nbits));
	printf("LENGTH_VALUE: %d\n", length_tree_fixed[node_index].symbol);
	printf("\n");

	node_index = 0;
	length_code = 0;
	for(int i = 0; i < 18; i++){
		bit = readbit(data, bit_index++);
		length_code = (length_code << 1) | bit;
		nbits = i + 1;
		if(bit == 0){
			node_index = node_index * 2 + 1;
		} else {
			node_index = node_index * 2 + 2;
		}
		if(length_tree_fixed[node_index].is_leaf){
			break;
		}
	}
	printf("LENGTH_CODE_LENGTH: %d\n", nbits);
	printf("LENGTH_CODE: %s\n", tobin(length_code, nbits));
	printf("LENGTH_VALUE: %d\n", length_tree_fixed[node_index].symbol);
	printf("\n");

	symbol = length_tree_fixed[node_index].symbol;
	assert(length_symbol_has_extra_bits(symbol));

	extra_bits_count = extra_bits_count_for_length_symbol(symbol);

	extra_bits = 0;
	for(int i = 0; i < extra_bits_count; i++){
		extra_bits = (extra_bits<<1) | readbit(data, bit_index++);
	}

	printf("EXTRA_BITS_COUNT: %d\n", extra_bits_count);
	printf("EXTRA_BITS: %s\n", tobin(extra_bits, extra_bits_count));

	length = length_from_length_symbol_and_extra_bits(symbol, extra_bits);

	printf("ACTUAL LENGTH: %d\n", length);


	distance_code = 0;
	for(int i = 0; i < 5; i++){
		distance_code = (distance_code << 1) | readbit(data, bit_index++);
	}

	printf("DISTANCE_CODE: %d\n", distance_code);

	node_index = 0;
	length_code = 0;
	for(int i = 0; i < 18; i++){
		bit = readbit(data, bit_index++);
		length_code = (length_code << 1) | bit;
		nbits = i + 1;
		if(bit == 0){
			node_index = node_index * 2 + 1;
		} else {
			node_index = node_index * 2 + 2;
		}
		if(length_tree_fixed[node_index].is_leaf){
			break;
		}
	}
	printf("LENGTH_CODE_LENGTH: %d\n", nbits);
	printf("LENGTH_CODE: %s\n", tobin(length_code, nbits));
	printf("LENGTH_VALUE: %d\n", length_tree_fixed[node_index].symbol);
	printf("\n");
}


int main(int argc, char *argv[]){
	//block(deflated);
	//
	int b;


	build_fixed_trees();
	example1();

//	inflate(data);

/*
	printf("0xab: %s\n", tobin(data[0], 8));
	b = readbit(data, 0);
	printf("BFINAL: %s\n", tobin(b, 1));
	b = 0;
	b |= readbit(data, 1) << 0;
	b |= readbit(data, 2) << 1;
	printf("BTYPE: %s\n", tobin(b, 2));

	b = 0;
	int node_index = 0;
	for(int i = 0; i < 8; i++){
		int x = readbit(data, i+3);
		b |= x << (7 - i);
		if(x == 0){
			node_index = node_index * 2 + 1;
		} else {
			node_index = node_index * 2 + 2;
		}
	}
	printf("symbol: %s\n", tobin(b, 8));
	printf("value: %d\n", length_tree_fixed[node_index].value);
*/
	return 0;
}

void inflate(uint8_t *data){
	int bit_index = 0;

	int bfinal = 0;
	int btype = 0;
	int len;
	int nlen;
	int node_index;
	int length;
	int distance;
	int code;
	int value;
	int length_extra_bits_count;
	int length_extra_bits;
	int distance_extra_bits_count;
	int distance_extra_bits;


	// RFC1951 "3.2.3. Details of block format"
	do {
		bfinal = readbit(data, bit_index++);
		btype = 0;
		btype |= readbit(data, bit_index++) << 0;
		btype |= readbit(data, bit_index++) << 1;

		switch(btype){
		case 0: // "00 - no compression"
			// RFC1951 "3.2.4. Non-compressed blocks (BTYPE=00)"

			// "Any bits of input up to the next byte boundary are ignored."
			if(bit_index % 8 != 0){
				bit_index += 8 - (bit_index % 8);
			}

			len = 0;
			len = data[bit_index/8 + 0] << 8;
			len = data[bit_index/8 + 1] << 0;
			bit_index += 16;

			nlen = 0;
			nlen = data[bit_index/8 + 0] << 8;
			nlen = data[bit_index/8 + 1] << 0;
			bit_index += 16;

			if(len != ~nlen){
				fflush(stdout);
				fprintf(stderr, "LEN != NLEN\n");
				exit(1);
			}

			fwrite(&data[bit_index/8], sizeof(uint8_t), len, stdout);
			bit_index += 8 * len;
			break;
		case 1: // "01 - compressed with fixed Huffman codes"

			while(1){
				node_index = 0;
				while(!length_tree_fixed[node_index].is_leaf){
					if(readbit(data, bit_index++) == 0){
						node_index = node_index * 2 + 1;
					} else {
						node_index = node_index * 2 + 2;
					}
				}

				value = length_tree_fixed[node_index].symbol;
				if(value < 256){
					printf("Literal: %c\n", value);
				} else if(257 <= value && value <= 285){
					length_extra_bits_count = extra_bits_count_for_length_symbol(value);
					length_extra_bits = 0;
					for(int i = 0; i < length_extra_bits_count; i++){
						length_extra_bits_count <<= 1;
						length_extra_bits_count |= readbit(data, bit_index++);
					}

					code = 0;
					for(int i = 0; i < 5; i++){
						code <<= 1;
						code |= readbit(data, bit_index++);
					}

					distance_extra_bits_count = extra_bits_for_distance_code(code);

					distance_extra_bits = 0;
					for(int i = 0; i < 5; i++){
						distance_extra_bits <<= 1;
						distance_extra_bits |= readbit(data, bit_index++);
					}


					// TODO: read distance;
				}
			}

			break;
		case 2: // "10 - compressed with dynamic Huffman codes"
			fflush(stdout);
			fprintf(stderr, "not implemented!\n");
			exit(1);
			break;
		case 3: // "11 - reserved (error)"
			fflush(stdout);
			fprintf(stderr, "reserved BTYPE type (error)\n");
			exit(1);
		default: // this case should never happen.
			fflush(stdout);
			fprintf(stderr, "programmer error\n");
			exit(1);
		};
	} while(bfinal == 0);
}

