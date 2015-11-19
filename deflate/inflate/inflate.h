/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#ifndef INFLATE_H
#define INFLATE_H

#include <stdint.h>

int inflate(uint8_t *input, int input_len, int *input_used, uint8_t *output, int output_size, int *output_len);

#endif
