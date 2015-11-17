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
struct tree_node encode_tree[524288];
struct tree_node lit_tree[524288];
struct tree_node dist_tree[524288];

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

int extra_bits_count_for_distance_code(int code){
	if(0 <= code && code <= 29){
		return distance_extra_bits[code][0];
	} else {
		fflush(stdout);
		fprintf(stderr, "invalid distance code\n");
		exit(1);
	}
}

int distance_from_code_and_extra(int code, int extra){
	if(0 <= code && code <= 29){
		return distance_extra_bits[code][1] + extra;
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

int is_length_literal_symbol_literal(int value){
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

	printf("LENGTH_CODE: %s (lenght: %d)\n", tobin(length_code, nbits), nbits);
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


void example2(){
	uint8_t *data = (uint8_t *)"\x4b\x4c\x4a\xc4\x80\x00";
	int bit_index;
	int bfinal;
	int btype;
	int node_index;
	int bit;
	int length_code;
	int nbits;
	int symbol;
	int extra_bits_count;
	int extra_bits;
	int length;
	int distance_code;
	int distance;
	int distance_extra_bits_count;
	int distance_extra_bits;


	bit_index = 0;
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


	// read second length code;
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


	// read third length code;
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


	// read fourth length code;
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
	distance_extra_bits_count = extra_bits_count_for_distance_code(distance_code);

	assert(distance_extra_bits_count == 0);


}

int read_block_header(uint8_t *data, int bit_index, int *bfinal, int *btype){
	int index;

	*bfinal = readbit(data, bit_index++);
	*btype = 0;
	*btype |= readbit(data, bit_index++) << 0;
	*btype |= readbit(data, bit_index++) << 1;
	return bit_index;
}

int read_length_literal_symbol(uint8_t *data, int bit_index, struct tree_node *tree, int *length_literal_symbol){
	int node_index;

	node_index = 0;
	for(int i = 0; i < 18; i++){
		if(readbit(data, bit_index++) == 0){
			node_index = node_index * 2 + 1;
		} else {
			node_index = node_index * 2 + 2;
		}
		if(tree[node_index].is_leaf){
			*length_literal_symbol = tree[node_index].symbol;
			return bit_index;
		}
	}

	fflush(stdout);
	fprintf(stderr, "invalid code > 18 bits\n");
	exit(1);
}

void example3(){
	uint8_t *data = (uint8_t *)"\x4b\x4c\x4a\xc4\x80\x00";
	int bit_index = 0;

	int bfinal = 0;
	int btype = 0;
	int length_literal_symbol = 0;

	bit_index = read_block_header(data, bit_index, &bfinal, &btype);
	assert(bfinal == 1);
	assert(btype == 1);

	bit_index = read_length_literal_symbol(data, bit_index, length_tree_fixed, &length_literal_symbol);
	printf("ll_symbol: %d\n", length_literal_symbol);
	if(is_length_literal_symbol_literal(length_literal_symbol)){
		;
	}
}

void build_tree(int *lengths, int nlengths, struct tree_node *tree, int nnodes);

void example4(){
	uint8_t data[] = {
		0x9d, 0x93, 0x5d, 0x6e, 0xdc, 0x30, 0x0c, 0x84, 0xaf, 0xc2, 0x03, 0x2c,
		0x7c, 0x87, 0x22, 0x7d, 0x29, 0x90, 0x16, 0x05, 0x8a, 0x1e, 0x80, 0x91,
		0xb8, 0x5e, 0x02, 0xfa, 0x71, 0x48, 0xca, 0xe7, 0xcf, 0xc8, 0xeb, 0xcd,
		0xa6, 0xaf, 0x7d, 0x13, 0x20, 0x91, 0x1c, 0x7e, 0x33, 0x7a, 0xed, 0x26,
		0x95, 0x74, 0xf3, 0x51, 0x29, 0xf7, 0xd2, 0x8d, 0x5c, 0x83, 0xb8, 0x4a,
		0x5c, 0x28, 0xf5, 0xe6, 0x92, 0x42, 0x62, 0x18, 0x71, 0xd6, 0x4d, 0x3d,
		0x69, 0x5b, 0x49, 0x8a, 0xc6, 0x42, 0xdf, 0x7b, 0x93, 0x44, 0x4c, 0x59,
		0xb9, 0x52, 0xc1, 0xb3, 0xe1, 0x0b, 0xfd, 0x91, 0xfc, 0x59, 0x7f, 0x36,
		0xad, 0x3c, 0x4c, 0x71, 0xf5, 0x93, 0x25, 0x49, 0x63, 0x9f, 0x5d, 0xd7,
		0x21, 0x54, 0x74, 0x1d, 0x85, 0x89, 0x13, 0xbd, 0x0f, 0x74, 0xd8, 0x75,
		0x17, 0x33, 0xa6, 0xd9, 0xf4, 0x9f, 0xb9, 0x2d, 0x84, 0x6e, 0xd2, 0xb2,
		0x89, 0x3d, 0xc7, 0x4a, 0x50, 0xed, 0xa5, 0xa8, 0xdf, 0x35, 0x2f, 0xf4,
		0xdb, 0x58, 0x5c, 0x5a, 0x10, 0x6e, 0x0e, 0x45, 0xb2, 0xe2, 0x54, 0xf4,
		0x4d, 0xac, 0x13, 0xce, 0x1e, 0x98, 0x5c, 0x39, 0x02, 0x25, 0x8f, 0x05,
		0x31, 0x33, 0x58, 0x88, 0x07, 0xe4, 0x2c, 0xf4, 0x0b, 0x22, 0x42, 0x5b,
		0xd2, 0x3c, 0xd0, 0xe6, 0x14, 0x29, 0x4d, 0xeb, 0x85, 0x46, 0xd0, 0xd6,
		0x2d, 0x98, 0x80, 0x07, 0xb0, 0x0a, 0x03, 0x83, 0xf2, 0x57, 0x99, 0x0f,
		0x1c, 0x78, 0x79, 0xce, 0x74, 0x90, 0x60, 0x4b, 0x83, 0x76, 0xb9, 0x69,
		0x9a, 0x9b, 0x8e, 0x12, 0xa6, 0x49, 0xc5, 0x01, 0xad, 0xf5, 0x46, 0x81,
		0x96, 0xdd, 0x16, 0x7a, 0xfd, 0x5f, 0x07, 0xbe, 0x49, 0x13, 0x6e, 0x53,
		0xdd, 0x6a, 0xbc, 0x6b, 0x3e, 0xf5, 0x2d, 0xf4, 0x37, 0x08, 0x96, 0xc1,
		0x2e, 0xba, 0xc2, 0x2b, 0xbf, 0xd0, 0x36, 0xca, 0xae, 0x8d, 0xe1, 0x22,
		0xb9, 0xd4, 0x4d, 0x60, 0xb2, 0xe4, 0xcb, 0x57, 0x4f, 0x35, 0x7f, 0x92,
		0x94, 0x52, 0x00, 0x52, 0xfc, 0x1d, 0x26, 0xf1, 0x48, 0xd0, 0x48, 0x4d,
		0x5d, 0x09, 0x4f, 0x2a, 0xaf, 0xed, 0xdc, 0x1b, 0xa6, 0x05, 0x39, 0xaf,
		0x3a, 0x91, 0x2e, 0xf4, 0x32, 0x8c, 0xdf, 0x74, 0x06, 0x25, 0xf3, 0xa6,
		0x6f, 0xc3, 0x69, 0xa2, 0x7b, 0xa2, 0x9e, 0x99, 0xa1, 0xed, 0xc6, 0x26,
		0x01, 0x93, 0x9f, 0x9c, 0xaf, 0x32, 0x56, 0x45, 0x27, 0x4c, 0x28, 0xa4,
		0x53, 0x5a, 0x56, 0x41, 0xba, 0xb0, 0x02, 0x2c, 0xd8, 0xf9, 0xf0, 0xf8,
		0x44, 0xaa, 0xd8, 0xd5, 0x30, 0xff, 0x0e, 0x72, 0x72, 0x4c, 0x69, 0x54,
		0xe7, 0xf6, 0xa0, 0x3f, 0x99, 0xf7, 0xac, 0x30, 0xdc, 0x3a, 0x34, 0xdd,
		0x13, 0xfa, 0x08, 0x56, 0x55, 0x04, 0x0d, 0x48, 0x8e, 0xb4, 0x3d, 0x81,
		0x20, 0x48, 0x5c, 0xc4, 0x07, 0x03, 0xdf, 0xe1, 0x97, 0xdd, 0x7a, 0x4b,
		0x33, 0xcb, 0x2f, 0xf8, 0x11, 0xde, 0x93, 0xa2, 0xa6, 0x71, 0xf4, 0x89,
		0x63, 0x43, 0x80, 0xe3, 0xbe, 0xdd, 0xac, 0x5b, 0xa1, 0x1a, 0x69, 0x73,
		0xda, 0xd8, 0xb0, 0xba, 0xce, 0xfc, 0xd5, 0x3e, 0xd9, 0x5d, 0x50, 0xe2,
		0xe9, 0xf8, 0x38, 0xa6, 0x79, 0xfa, 0x0f, 0x22, 0x75, 0x76, 0xfd, 0xd1,
		0xc8, 0x46, 0x18, 0x7a, 0x3f, 0xd4, 0x3f, 0x83, 0x31, 0x3f, 0xc9, 0xfc,
		0x2c, 0x67, 0x30, 0x0f, 0xea, 0x93, 0x32, 0x32, 0x77, 0x45, 0xe6, 0x60,
		0x25, 0xee, 0x4e, 0x03, 0xf1, 0x6f, 0xc0, 0x03, 0xb9, 0x5e, 0x3e, 0x00
	};

	int bit_index = 0;

	int bfinal = 0;
	int btype = 0;

	bfinal = readbit(data, bit_index++);
	btype = readbit(data, bit_index++);
	btype |= readbit(data, bit_index++) << 1;

	printf("BFINAL: %s\n", tobin(bfinal, 1));
	printf("BTYPE: %s\n", tobin(btype, 2));

	int hlit = 0;
	int hdist = 0;
	int hclen = 0;

	hlit = 0;
	hlit |= readbit(data, bit_index++);
	hlit |= readbit(data, bit_index++) << 1;
	hlit |= readbit(data, bit_index++) << 2;
	hlit |= readbit(data, bit_index++) << 3;
	hlit |= readbit(data, bit_index++) << 4;
	hdist |= readbit(data, bit_index++);
	hdist |= readbit(data, bit_index++) << 1;
	hdist |= readbit(data, bit_index++) << 2;
	hdist |= readbit(data, bit_index++) << 3;
	hdist |= readbit(data, bit_index++) << 4;
	hclen |= readbit(data, bit_index++);
	hclen |= readbit(data, bit_index++) << 1;
	hclen |= readbit(data, bit_index++) << 2;
	hclen |= readbit(data, bit_index++) << 3;

	printf("HLIT: %s (%d)\n", tobin(hlit, 5), hlit);
	printf("HDIST: %s (%d)\n", tobin(hdist, 5), hdist);
	printf("HCLEN: %s (%d)\n", tobin(hclen, 4), hclen);

	assert(hlit + 257 == 276);
	assert(hdist + 1 == 20);
	assert(hclen + 4 == 16);

	int debug_lengths[19] = {6, 5, 4, 3, 3, 4, 3, 3, 0, 3, 0, 3, 0, 4, 0, 6, 0, 0, 0};
	int lengths_order[19] = { 16, 17, 18,  0,  8,  7,  9,  6, 10,  5, 11, 4, 12,  3, 13,  2, 14,  1, 15, };
	int enc_lengths[19] = {0};

	for(int i = 0; i < hclen+4; i++){
		int a, b, c;
		a = readbit(data, bit_index++);
		b = readbit(data, bit_index++);
		c = readbit(data, bit_index++);
		int len = a | b<<1 | c <<2;
		//len = a<<2 | b<<1 | c;
		printf("LEN%d: %s (%d)\n", i, tobin(len, 3), len);
		assert(len == debug_lengths[i]);

		enc_lengths[lengths_order[i]] = len;
	}

/* lengths:
 16 17 18  0  8  7  9  6 10  5 11 4 12  3 13  2 14  1 15
  6  5  4  3  3  4  3  3  0  3  0 3  0  4  0  6  0  0  0

 0:3
 1:0
 2:6
 3:4
 4:3
 5:3
 6:3
 7:4
 8:3
 9:3
10:0
11:0
12:0
13:0
14:0
15:0
16:6
17:5
18:4
19:0

*/

/*
 *

FROM DEBUGGING puff.c
N   symbol count
--  ------ ------
 0   0     7
 1   4     0
 2   5     0
 3   6     6
 4   8     3
 5   9     1
 6   3     2
 7   7     0
 8  18     0
 9  17     0
10   2     0
11  16     0


Symbol Lenght   Code
------ ------   ----
 0      3       000
 2      6       111110
 3      4       1100
 4      3       001
 5      3       010
 6      3       011
 7      4       1101
 8      3       100
 9      3       101
16      6       111111
17      5       11110
18      4       1110

*/


	build_tree(enc_lengths, sizeof(enc_lengths)/sizeof(enc_lengths[0]), encode_tree, sizeof(encode_tree)/sizeof(encode_tree[0]));

	struct enc_codes {
		int symbol;
		char *code;
	};

	struct enc_codes codes_table[] = {
		{ 0, "000"},
		{ 2, "111110"},
		{ 3, "1100"},
		{ 4, "001"},
		{ 5, "010"},
		{ 6, "011"},
		{ 7, "1101"},
		{ 8, "100"},
		{ 9, "101"},
		{16, "111111"},
		{17, "11110"},
		{18, "1110"},
	};

	int node_index = 0;
	//memset(encode_tree, 0, sizeof(encode_tree));

	for(int i = 0; i < 12; i++){
		node_index = 0;
		char *code = codes_table[i].code;
		for(int j = 0; j < strlen(code); j++){
			assert(encode_tree[node_index].is_leaf == 0);
			if(code[j] == '0'){
				node_index = node_index * 2 + 1;
			} else {
				node_index = node_index * 2 + 2;
			}
		}
		/*
		encode_tree[node_index].is_leaf = 1;
		encode_tree[node_index].symbol = codes_table[i].symbol;
		*/

		printf("Testing: %d", i);
		fflush(stdout);
		assert(encode_tree[node_index].is_leaf == 1);
		assert(encode_tree[node_index].symbol == codes_table[i].symbol);
		printf("... OK\n");
	}

	printf("tree is build\n");

/*
SYM: 18 [21:(32)]
SYM:  4
SYM: 18 [0:(11)]
SYM:  7
SYM:  0
...
*/

int symbol_order[] = {
	18, 4, 18, 7, 0, 6, 18, 9, 0,
	8, 9, 17, 9, 0, 0, 8, 9, 9,
	0, 9, 0, 0, 9, 0, 9, 18, 4,
	8, 5, 5, 4, 7, 6, 7, 4, 0,
	0, 5, 16, 6, 8, 4, 4, 4, 5,
	6, 18, 8, 3, 5, 6, 7, 6, 7,
	8, 7, 8, 9, 17, 9, 17, 5, 0,
	0, 5, 5, 4, 4, 4, 3, 4, 2,
	3, 3, 5,

};
int symbol_i = 0;

	int lengths[318];

	for(int i = 0; i < hlit+257 + hdist+1; i++){
		int stop = 0;
		node_index = 0;
		do {
			assert(encode_tree[node_index].is_leaf == 0);
			int bit = readbit(data, bit_index++);
			if(bit == 0){
				node_index = node_index * 2 + 1;
			} else {
				node_index = node_index * 2 + 2;
			}
			stop += 1;
			assert(stop < 18);
		} while(encode_tree[node_index].is_leaf == 0);
		int symbol = encode_tree[node_index].symbol;
		assert(symbol == symbol_order[symbol_i++]);

		if(0 <= symbol && symbol <= 15){
			printf("\t('lit', %d),\n", symbol);
			lengths[i] = symbol;
		} else if(symbol == 16){
			int repeat = 0;
			int extra = 0;
			extra = readbit(data, bit_index++) << 0;
			extra |= readbit(data, bit_index++) << 1;
			repeat = extra + 3;
			printf("\t('rep', %d),\n", repeat);
			if(i == 0){
				fflush(stdout);
				fprintf(stderr, "can't repeat last when there isn't a last!\n");
				exit(1);
			}
			int last_length = lengths[i-1];
			for(int j = 0; j < repeat; j++){
				lengths[i+j] = last_length;
			}
			i += repeat - 1;
		} else if(symbol == 17){
			int zeros = 0;
			int extra = 0;
			extra = readbit(data, bit_index++) << 0;
			extra |= readbit(data, bit_index++) << 1;
			extra |= readbit(data, bit_index++) << 2;
			zeros = extra + 3;
			printf("\t('repz', %d),\n", zeros);
			for(int j = 0; j < zeros; j++){
				lengths[i+j] = 0;
			}
			i += zeros - 1;
		} else if(symbol == 18){
			int zeros = 0;
			int extra = 0;
			extra = readbit(data, bit_index++) << 0;
			extra |= readbit(data, bit_index++) << 1;
			extra |= readbit(data, bit_index++) << 2;
			extra |= readbit(data, bit_index++) << 3;
			extra |= readbit(data, bit_index++) << 4;
			extra |= readbit(data, bit_index++) << 5;
			extra |= readbit(data, bit_index++) << 6;
			zeros = extra + 11;
			printf("\t('repz', %d),\n", zeros);
			for(int j = 0; j < zeros; j++){
				lengths[i+j] = 0;
			}
			i += zeros - 1;
		}
	}
	printf("bits: %d bytes: %d\n", bit_index, bit_index/8);
	printf("symbol_i: %d / %lu\n", symbol_i, sizeof(symbol_order) / sizeof(int));

	build_tree(lengths, hlit+257, lit_tree, sizeof(lit_tree) / sizeof(lit_tree[0]));
	build_tree(lengths + hlit + 257, hdist+1, dist_tree, sizeof(dist_tree) / sizeof(dist_tree[0]));

/*
 *
$ ./z.py  | ./puff | grep LENGHT | sort | uniq | awk -F, '{ print $2,$1 }' OSF=, -
257 000
32 0010
97 0011
101 0100
105 0101
114 0110
115 0111
116 1000
99 10010
100 10011
108 10100
109 10101
110 10110
111 10111
117 11000
258 11001
46 110100
103 110101
112 110110
118 110111
259 111000
261 111001
44 1110100
102 1110101
104 1110110
260 1110111
262 1111000
264 1111001
67 11110100
76 11110101
98 11110110
113 11110111
256 11111000
263 11111001
265 11111010
65 111110110
68 111110111
73 111111000
77 111111001
78 111111010
80 111111011
83 111111100
85 111111101
266 111111110
275 111111111
*/

	struct enc_codes lit_codes_table[] = {
		{ 257, "000"},
		{  32, "0010"},
		{  97, "0011"},
		{ 101, "0100"},
		{ 105, "0101"},
		{ 114, "0110"},
		{ 115, "0111"},
		{ 116, "1000"},
		{  99, "10010"},
		{ 100, "10011"},
		{ 108, "10100"},
		{ 109, "10101"},
		{ 110, "10110"},
		{ 111, "10111"},
		{ 117, "11000"},
		{ 258, "11001"},
		{  46, "110100"},
		{ 103, "110101"},
		{ 112, "110110"},
		{ 118, "110111"},
		{ 259, "111000"},
		{ 261, "111001"},
		{  44, "1110100"},
		{ 102, "1110101"},
		{ 104, "1110110"},
		{ 260, "1110111"},
		{ 262, "1111000"},
		{ 264, "1111001"},
		{  67, "11110100"},
		{  76, "11110101"},
		{  98, "11110110"},
		{ 113, "11110111"},
		{ 256, "11111000"},
		{ 263, "11111001"},
		{ 265, "11111010"},
		{  65, "111110110"},
		{  68, "111110111"},
		{  73, "111111000"},
		{  77, "111111001"},
		{  78, "111111010"},
		{  80, "111111011"},
		{  83, "111111100"},
		{  85, "111111101"},
		{ 266, "111111110"},
		{ 275, "111111111"},
	};
	//memset(lit_tree, 0, sizeof(lit_tree));

	for(int i = 0; i < 45; i++){
		node_index = 0;
		char *code = lit_codes_table[i].code;
		for(int j = 0; j < strlen(code); j++){
			assert(lit_tree[node_index].is_leaf == 0);
			if(code[j] == '0'){
				node_index = node_index * 2 + 1;
			} else {
				node_index = node_index * 2 + 2;
			}
		}
		//lit_tree[node_index].is_leaf = 1;
		//lit_tree[node_index].symbol = lit_codes_table[i].symbol;
		assert(lit_tree[node_index].is_leaf == 1);
		assert(lit_tree[node_index].symbol == lit_codes_table[i].symbol);
	}
	printf("LIT TREE BUILT!\n");

/*
$ ./z.py  | ./puff | grep DIST | sort | uniq | awk -F, '{ print $2,$1 }' OSF=, -
16 00
14 010
17 011
18 100
11 1010
12 1011
13 1100
15 1101
6 11100
9 11101
10 11110
19 11111
*/

	struct enc_codes dist_codes_table[] = {
		{16, "00"},
		{14, "010"},
		{17, "011"},
		{18, "100"},
		{11, "1010"},
		{12, "1011"},
		{13, "1100"},
		{15, "1101"},
		{ 6, "11100"},
		{ 9, "11101"},
		{10, "11110"},
		{19, "11111"},
	};

	//memset(dist_tree, 0, sizeof(dist_tree));

	for(int i = 0; i < 12; i++){
		node_index = 0;
		char *code = dist_codes_table[i].code;
		for(int j = 0; j < strlen(code); j++){
			assert(dist_tree[node_index].is_leaf == 0);
			if(code[j] == '0'){
				node_index = node_index * 2 + 1;
			} else {
				node_index = node_index * 2 + 2;
			}
		}
		//dist_tree[node_index].is_leaf = 1;
		//dist_tree[node_index].symbol = dist_codes_table[i].symbol;
		assert(dist_tree[node_index].is_leaf == 1);
		assert(dist_tree[node_index].symbol == dist_codes_table[i].symbol);
	}
	printf("DIST TREE BUILT!\n");



	int lit = 0;

	do {
		node_index = 0;
		do {
			int bit = readbit(data, bit_index++);
			if(bit == 0){
				node_index = node_index * 2 + 1;
			} else {
				node_index = node_index * 2 + 2;
			}
		}while(!lit_tree[node_index].is_leaf);

		int symbol = lit_tree[node_index].symbol;

		if (0 <= symbol && symbol <= 255){
			printf("\t('lit', '%c'),\n", symbol);
		} else if(symbol == 256){
			break;
		} else if(257 <= symbol && symbol <= 285){
			int length;

			int nextra;
			int extra;

			nextra = extra_bits_count_for_length_symbol(symbol);
			extra = 0;
			for(int i = 0; i < nextra; i++){
				extra = (extra << 1) | readbit(data, bit_index++);
			}

			length = length_from_length_symbol_and_extra_bits(symbol, extra);


			int distance;

			node_index = 0;
			do {
				int bit = readbit(data, bit_index++);
				if(bit == 0){
					node_index = node_index * 2 + 1;
				} else {
					node_index = node_index * 2 + 2;
				}
			}while(!dist_tree[node_index].is_leaf);

			symbol = dist_tree[node_index].symbol;

			nextra = extra_bits_count_for_distance_code(symbol);
			extra = 0;
			for(int i = 0; i < nextra; i++){
				int bit = readbit(data, bit_index++);
				extra |= bit << i;
				printf("#\tbit: %d extra: _%d_\n", bit, extra);
			}

			distance = distance_from_code_and_extra(symbol, extra);


			printf("#\t sym: %d extra: %d nextra: %d\n", symbol, extra, nextra);
			printf("\t('ld', %d, %d),\n", length, distance);


		} else {
			fflush(stdout);
			fprintf(stderr, "invalid symbol\n");
			exit(1);
		}

	} while(1);

}

void build_tree(int *lengths, int nlengths, struct tree_node *tree, int nnodes){
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

	printf("After step 1:\n");
	printf("\n");
	printf("N      bl_count[N]\n");
	printf("-      -----------\n");
	for(int i = 0; i < 16; i++){
		if(0 < bl_count[i]){
			printf("%d     %d\n", i, bl_count[i]);
		}
	}
	printf("\n\n");

	int code = 0;
	bl_count[0] = 0;
	for(int bits = 1; bits < 16; bits++){
		code = (code + bl_count[bits - 1]) << 1;
		next_code[bits] = code;
	}

	printf("After step 2:\n");
	printf("\n");
	printf("N      next_code[N]\n");
	printf("-      ------------\n");
	for(int i = 0; i < 16; i++){
		if(0 < next_code[i] && bl_count[i] != 0){
			printf("%d     %d\n", i, next_code[i]);
		}
	}

	memset(tree, 0, sizeof(*tree) * nnodes);

	for(int i = 0; i < nlengths; i++){
		int l = lengths[i];
		if(l != 0){
			int code = next_code[l];
			next_code[l] += 1;

			printf("%-3d:", i);
			int node_index = 0;
			for(int j = l - 1; 0 <= j; j--){
				int bit = (code >> j) & 1;
				printf("%c", bit == 0 ? '0' : '1');
				fflush(stdout);
				assert(tree[node_index].is_leaf == 0);
				if(bit == 0){
					node_index = node_index * 2 + 1;
				} else {
					node_index = node_index * 2 + 2;
				}
			}
			tree[node_index].is_leaf = 1;
			tree[node_index].symbol = i;
			printf("\n");
		}
	}
}


void example5(){

	int lengths[] = {3, 3, 3, 3, 3, 2, 4, 4};

	build_tree(lengths, sizeof(lengths)/sizeof(lengths[0]), dist_tree, sizeof(dist_tree)/sizeof(dist_tree[0]));

}

void example6(){
	int lengths[] = {3, 0, 6, 4, 3, 3, 3, 4, 3, 3, 0, 0, 0, 0, 0, 0, 6, 5, 4, 0};

	build_tree(lengths, sizeof(lengths)/sizeof(lengths[0]), dist_tree, sizeof(dist_tree)/sizeof(dist_tree[0]));
}

void inflate(uint8_t *data, uint8_t *output);

int main(int argc, char *argv[]){
	//block(deflated);
	//
	int b;


	build_fixed_trees();
#if 1
	//example1();
	//example2();
	//example3();
	example4();
	//example5();
	//example6();

#else
	uint8_t *data = (uint8_t *)"\x01\x0c\x00\xf3\xff\x48\x65\x6c\x6c\x6f\x20\x77\x6f\x72\x6c\x64\x21";
	uint8_t buff[1024] = {0};
	inflate(data, buff);
	printf("%s\n", buff);
#endif

	return 0;
}

void inflate(uint8_t *data, uint8_t *output){
	uint8_t *pout = output;
	int bit_index = 0;

	int bfinal = 0;
	int btype = 0;

	// RFC1951 "3.2.3. Details of block format"
	do {
		bfinal = readbit(data, bit_index++);
		btype = readbit(data, bit_index++);
		btype |= readbit(data, bit_index++) << 1;

		switch(btype){
		case 0: // "00 - no compression"
			{ // RFC1951 "3.2.4. Non-compressed blocks (BTYPE=00)"
				uint16_t len;
				uint16_t nlen;

				// "Any bits of input up to the next byte boundary are ignored."
				if(bit_index % 8 != 0){
					bit_index += 8 - (bit_index % 8);
				}

				len = data[bit_index/8];
				len |= data[bit_index/8 + 1] << 8;
				bit_index += 16;

				nlen = data[bit_index/8];
				nlen |= data[bit_index/8 + 1] << 8;
				bit_index += 16;

				if(len != (~nlen & 0xffff)){
					fflush(stdout);
					fprintf(stderr, "LEN != NLEN\n");
					exit(1);
				}

				for(int i = 0; i < len; i++){
					*pout = data[bit_index/8];
					bit_index += 8;
					pout++;
				}
			}
			break;
		case 1: // "01 - compressed with fixed Huffman codes"

			while(1){
				int symbol;
				int node_index;
				int length_code;
				int len_symbol;

				node_index = 0;
				do {
					int bit = readbit(data, bit_index++);
					if(bit == 0){
						node_index = node_index * 2 + 1;
					} else {
						node_index = node_index * 2 + 2;
					}
				}while(!length_tree_fixed[node_index].is_leaf);

				len_symbol = length_tree_fixed[node_index].symbol;

				if(0 <= len_symbol && len_symbol <= 255){		// "where values 0..255 represent literal bytes"
					printf("Literal: %c\n", len_symbol);
					*pout++ = len_symbol;
				} else if(len_symbol == 256){				// "the value 256 indicates end-of-block"
					break;
				} else if(257 <= len_symbol && len_symbol <= 285){	// "values 257..285 represent length codes"
					int nextra;
					int extra_bits;
					int length;
					int distance_code;
					int distance;

					nextra = extra_bits_count_for_length_symbol(len_symbol);
					extra_bits = 0;
					for(int i = 0; i < nextra; i++){
						extra_bits <<= 1;
						extra_bits |= readbit(data, bit_index++);
					}

					length = length_from_length_symbol_and_extra_bits(len_symbol, extra_bits);

					distance_code = 0;
					for(int i = 0; i < 5; i++){
						distance_code <<= 1;
						distance_code |= readbit(data, bit_index++);
					}

					nextra = extra_bits_count_for_distance_code(distance_code);

					extra_bits = 0;
					for(int i = 0; i < nextra; i++){
						extra_bits <<= 1;
						extra_bits |= readbit(data, bit_index++);
					}

					distance = distance_from_code_and_extra(distance_code, extra_bits);

					printf("Length Distance < %d, %d >\n", length, distance);
					for(int i = 0; i < length; i++){
						*pout = *(pout - distance);
						pout++;
					}
				} else if(286 <= len_symbol && len_symbol <= 287){
					/* RFC1951:
					 * "Literal/length values 286-287 will never actually
					 * occur in the compressed data, but participate in the code
					 * construction."
					 */
					fflush(stdout);
					fprintf(stderr, "invalid length symbol for fixed codes\n");
					exit(1);
				} else {
					fflush(stdout);
					fprintf(stderr, "invalid length symbol (programmer error?)\n");
					exit(1);
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

