// Erik Schluter
// 11/9/2014

#include "arb_int.h"

#define SUCCESS 1;
#define FAIL 0
#define MAX_LONG_DIGITS 21

// These three functions are only used within the implementation functions below
// so I keep them outside the header
static void switch_sign (arb_int_t x);
static void copy_data (const arb_int_t x, char * dest);
static void little_add (char * x, const int xLen, char * y, const int yLen);

// Type desctructor
void arb_free (arb_int_t i) {
	
	free (i->handle);
	free (i);
}

arb_int_t arb_duplicate (const arb_int_t i) {

	int j;
	arb_int_t ret;

	ret = (data *) malloc ( sizeof(data) );
	
	ret->handle = malloc ( i->num_bytes );

	for (j = 0; j < i->num_bytes; j++)
		ret->handle[j] = i->handle[j];

	ret->num_bytes = i->num_bytes;
	ret->sign      = i->sign;
	
	return ret;
}

int arb_from_string (arb_int_t * i, const char * s) {

	int k, j, start;
	int length;
	char temp[2];

	length = (int)strlen(s);

	(*i) = (data *) malloc ( sizeof(data) );

	switch ( s[0] ) {
		case '-':
			start = 1;
			(*i)->sign = '-';
			break;
		case '+':
			start = 1;
			(*i)->sign = '+';
			break;
		default:
			start = 0;
			(*i)->sign = '+';
			break;
	}

	if ( length != 1 ) {
		while ( s[start] == '0' ) { start++; }

		if ( start == length ) {
			fprintf (stderr, "WARNING (in arb_from_string): 0 assumed from input string %s\n", s);
			start -= 1;
			(*i)->sign = '+';
		}
	}

	for (k = start; k < length; k++) {
		if ( !isdigit(s[k]) ) {
			fprintf (stderr, "(arb_from_string) Error: please supply a valid number string\n");
			free (*i);
			return FAIL;
		}
	}

	//assign decimal digits to a byte with values 0 - 9 (even though signed char max is 127)
	(*i)->handle     = malloc ( length - start );
	
	for (k = start, j = 0; k < length; k++, j++) {
		temp[0] = s[k];
		temp[1] = '\0';
		*((*i)->handle + j) = atoi(temp);
	}
	
	(*i)->num_bytes = j;

	return SUCCESS;
}

int arb_from_int (arb_int_t * i, signed long int source) {

	char temp[MAX_LONG_DIGITS];

	sprintf(temp, "%ld", source);

	return arb_from_string (i, temp);
}

int arb_to_string (const arb_int_t i, char * buf, size_t max) {

	int j;
	int k = 0;

	if ( i->sign == '-' ) {
		buf[k++] = i->sign;
	}

	for (j = 0; (j < (i->num_bytes)) && (j < max-1); j++) {
		buf[k++] = i->handle[j] + 48;
	}
	buf[k] = '\0';

	if (j != i->num_bytes) {
		return FAIL;
	} else {
		return SUCCESS;
	}
}

int arb_to_int (const arb_int_t i, long int * out) {

	int ret;
	char * tempStr;
	char * endPtr = NULL;
	size_t max = i->num_bytes + 2;  // accomodate space for possible negative symbol and null term (add 2)
	
	tempStr = malloc ( max );

	ret = arb_to_string (i, tempStr, max);

	*out = strtol (tempStr, &endPtr, 10);

	free (tempStr);

	if (*out == LONG_MAX || *out == LONG_MIN) {
		fprintf (stderr, "(arb_to_int) Error code: %s\n", strerror(errno));
		return FAIL;
	}

	return ret;
}

void arb_add (arb_int_t x, const arb_int_t y) {
	
	int j, k, n, digSum, holdLen, xPlace, yPlace;
	int carry = 0;
	char * littleHold;
	arb_int_t switchY;

	if ( x->sign != y->sign ) {
		switchY = arb_duplicate (y);
		switch_sign (switchY);
		arb_subtract(x, switchY);
		arb_free (switchY);
		return;
	}

	if ( x->num_bytes >= y->num_bytes ) {  // Here we must find the max length of the two numbers and add one units place
		holdLen = 3 + x->num_bytes;   // It is only possible to increase length by 1 digit when adding two numbers of equal length
	} else {
		holdLen = 3 + y->num_bytes;
	}

	littleHold = malloc (holdLen - 1);

	for (j = x->num_bytes-1, k = y->num_bytes-1, n = 0; j >= 0 || k >= 0; j--, k--, n++) {
		xPlace = 0;
		yPlace = 0;
		if (j > -1) 
			xPlace = x->handle[j];
		if (k > -1)
			yPlace = y->handle[k];
		if ( (digSum = xPlace + yPlace + carry) > 9 ) {   // if the sum is two digit we need a carry
			littleHold[n] = digSum - 10;  // max sum for addition is 19
			carry	      = 1;
		} else {
			littleHold[n] = digSum;
			carry  	      = 0;
		}
	}
	
	if (carry) {
		littleHold[n] = carry;
		x->num_bytes = n + 1;
	} else {
		x->num_bytes = n;
	}

	x->handle = realloc ( x->handle, x->num_bytes );
	// switch the order of the array
	for (j = 0, k = (x->num_bytes - 1); k >= 0; j++, k--) {
		x->handle[j] = littleHold[k];
	}

	free (littleHold);
}

void arb_subtract (arb_int_t x, const arb_int_t y) {

	int i, j, n, holdLen, subValue, borrow, removeLead, upperSize, lowerSize;
	arb_int_t absY;
	char * littleHold;
	char * upper;
	char * lower;

	if ( arb_compare (x, y) == 0 ) {
		x->handle = realloc ( x->handle, 1 );
		x->handle[0] = 0;
		x->num_bytes = 1;
		x->sign      = '+';
		return;
	}

	// If the signs are different the subtraction operation is the same as adding
	if ( x->sign != y->sign ) {  
		absY = arb_duplicate(y);
		switch_sign (absY);
		arb_add (x, absY);
		arb_free (absY);
		return;
	}

	// Now that we have checked for the previous two conditions we can allocate memory for the operations
	if ( x->num_bytes > y->num_bytes ) {
		holdLen = 1 + x->num_bytes;
	} else {
		holdLen = 1 + y->num_bytes;
	}
	littleHold = malloc (holdLen);
	upper 	   = malloc (holdLen);
	lower 	   = malloc (holdLen);

	if ( x->sign == '+' && y->sign == '+' ) {
		if ( arb_compare (x, y) == -1 ) {
			x->sign = '-';
			copy_data (y, upper); upperSize = y->num_bytes; // Since we want to take the difference we can think of 
			copy_data (x, lower); lowerSize = x->num_bytes; // the two numbers as absolute value numbers. So we need
		} else {						// to figure out which one is bigger and assign that as
			x->sign = '+';					// the number which is subtracted from (upper) and the
			copy_data (x, upper); upperSize = x->num_bytes; // smaller number to lower.
			copy_data (y, lower); lowerSize = y->num_bytes;
		}
	} else {
		if ( ((-1)*arb_compare (x, y)) == -1 ) {
			x->sign = '+';
			copy_data (y, upper); upperSize = y->num_bytes;
			copy_data (x, lower); lowerSize = x->num_bytes;
		} else {
			x->sign = '-';
			copy_data (x, upper); upperSize = x->num_bytes;
			copy_data (y, lower); lowerSize = y->num_bytes;
		}
	}
	
	for (i = (upperSize - 1), j = (lowerSize - 1), n = 0; i >= 0; i--, j--, n++) {
		subValue = 0;
		if ( j > -1 ) subValue = lower[j];
		if ( subValue > upper[i] ) {
			borrow = i - 1;
			while ( upper[borrow] == 0 ) {
				upper[borrow] = 9;
				borrow--;
			}
			upper[borrow] -= 1;
			upper[i] += 10;
		}
		littleHold[n] = upper[i] - subValue;
	}

	//remove leading zeros
	removeLead = n-1;
	while ( (littleHold[removeLead] == 0) && (removeLead > 0) ) { removeLead--; }
	
	x->handle = realloc ( x->handle, (x->num_bytes = removeLead + 1) );

	// switch the order of the array
	for (i = 0, j = removeLead; j >= 0; i++, j--) {
		x->handle[i] = littleHold[j];
	}
	
	free (littleHold);
	free (upper);
	free (lower);
}

void arb_multiply (arb_int_t x, const arb_int_t y) {

	int i, j, n, prodLen; 
	char partialProd, carry; 
	char * littleHold;
	char * fullProd;
	int leadZero = 0;

	if ( x->sign != y->sign ) {
		x->sign = '-';
	} else {
		x->sign = '+';
	}

	prodLen    = x->num_bytes + y->num_bytes;      // the maximum number of digits in the product is the sum
	littleHold = calloc ( prodLen, sizeof(char) ); // of the digits across both numbers (min is this minus 1)
	fullProd   = malloc ( prodLen );

	for (i = (x->num_bytes-1); i >= 0; i--) {
		n     = 0;
		carry = 0;
		while( n < leadZero ) { fullProd[n++] = 0; }
		leadZero++;
		for (j = (y->num_bytes-1); j >= 0; j--) {
			partialProd = (x->handle[i]) * (y->handle[j]) + carry;
			carry = partialProd / 10;  // I use integer division to figure out the value in the 10's digit
			fullProd[n++] = partialProd - (carry * 10); // value in the 1's digit
		}
		fullProd[n++] = carry;  // final position is the last carry
		little_add (littleHold, prodLen, fullProd, n); // keep a running sum of the place products
	}

	if ( littleHold[prodLen-1] == 0 ) {
		x->num_bytes = prodLen - 1;
	} else {
		x->num_bytes = prodLen;
	}

	x->handle = realloc ( x->handle, x->num_bytes );

	for (i = 0, j = (x->num_bytes-1); j >= 0; i++, j--)
		x->handle[i] = littleHold[j];

	free (fullProd);
	free (littleHold);
}

int arb_compare (const arb_int_t x, const arb_int_t y) {

	int i = 0; 
	int invert = 1; 

	if ( x->sign != y->sign ) {
		if (x->sign == '-') {
			return (-1);
		} else {
			return 1;
		}
	}

	if ( x->sign == '-' && y->sign == '-' )  // invert return if both numbers being compared are negative
		invert = -1;

	if ( x->num_bytes < y->num_bytes ) {
		return ((-1) * invert);
	} else if ( x->num_bytes > y->num_bytes ) {
		return (1 * invert);
	} else {
		while ( (i < x->num_bytes && i < y->num_bytes) && (x->handle[i] == y->handle[i]) ) { i++; }

		if ( i == x->num_bytes && i == y->num_bytes ) {  // the two arb_ints are equal
			return 0;
		} else if ( x->handle[i] < y->handle[i] ) {
			return ((-1) * invert);
		} else {
			return (1 * invert);
		}
	}

	fprintf (stderr, "Error (arb_compare) : arb_ints unable to be compared. No function return possible...\n");
}

void switch_sign (arb_int_t x) {

	if ( x->sign == '-' ) {		// just switch the sign
		x->sign = '+';
	} else {
		x->sign = '-';
	}
}

void copy_data (const arb_int_t x, char * dest) {
	
	int i;

	for (i = 0; i < x->num_bytes; i++) {
		dest[i] = x->handle[i];
	}
}

// This function adds two little endian number strings
void little_add (char * x, const int xLen, char * y, const int yLen) {
	
	int j, xPlace, yPlace, digSum;
	int carry = 0;

	for (j = 0; j < xLen; j++) {	
		xPlace = x[j];
		yPlace = 0;
		if (j < yLen)
			yPlace = y[j];
		if ( (digSum = xPlace + yPlace + carry) > 9 ) {     // if the sum is two digit we need a carry
			x[j]   = digSum - 10;  // max sum for addition is 19
			carry  = 1;
		} else {
			x[j]   = digSum;
			carry  = 0;
		}
	}
}
