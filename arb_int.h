// Erik Schluter
// 11/9/2014

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

struct data {
	char * handle;
	int num_bytes;
	char sign;
};

typedef struct data data;
typedef data* arb_int_t;

void arb_free (arb_int_t i);
arb_int_t arb_duplicate (const arb_int_t i);

int arb_from_string (arb_int_t * i, const char * s);
int arb_from_int (arb_int_t * i, signed long int source);
int arb_to_string (const arb_int_t i, char * buf, size_t max);
int arb_to_int (const arb_int_t i, long int * out);

void arb_add (arb_int_t x, const arb_int_t y);
void arb_subtract (arb_int_t x, const arb_int_t y);
void arb_multiply (arb_int_t x, const arb_int_t y);
void arb_divide (arb_int_t x, const arb_int_t y);

int arb_compare (const arb_int_t x, const arb_int_t y);
