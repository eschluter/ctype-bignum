# Arbitrary Precision Integer Library

### arbitrary precision integer type (arb_int_t)
	
arb_int_t is a pointer to a structure named "data". "data" contains three members: handle, num_bytes, and sign. 'handle' is a char pointer which holds the digits of the integer; one digit (0-9) per character. I chose to use characters to hold my arbitrary integer digits because they only take up one byte of data as opposed to two bytes for a short and more for int. char is an integral type so arithmetic behaves as ints, shorts, etc. 'num_bytes' keeps track of how many digits are currently stored in 'handle'. 'sign' keeps track of the sign of the arb_int. I noticed that if you designed arb_int_t as the structure "data" instead of a pointer to that structure then you would run into trouble with arb_add/subtract/multiply because you wouldn't be able to modify x from within these functions without losing the changes when you exit. Even though a copy of the pointer x is being passed in these functions, the arb_int_t pointer in main and the local copy in arb_... point to the same "data" structure in memory and can modify it.

### arb_from_string
		
I first allocate data for the structure 'data' because when you declare arb_int_t you are only declaring a pointer to a 'data' structure. Then I check for the sign and assign the 'sign' member. I then check for leading zeros and  non-numeric characters (including '+' and '-' because there can only by one sign at the beginning of the number). I then allocate memory for the arb_int into 'handle' member and fill it with digits from the input string.

### arb_from_int

I first dump the long int into a string using sprintf and pass the resulting string to arb_from_string. The return from arb_from_string is returned.

### arb_to_string

I first check for the sign. Then I convert the integer values into their respective ascii codes by adding 48 (since character '0' is ascii value 48) and  drop them into buf. If the loop iterator doesn't reach all the bytes in 'handle' return FAIL.

### arb_to_int

I first allocate memory for a string then generate a string by calling arb_from_string. I use strtol to convert the string to a long int. strtol returns LONG_MAX or LONG_MIN when there is overflow. I check this and return the strtol error code defined in the errno header if overflow occurs.

### arb_free

Free's the bytes in 'handle' and then free's the structure 'data'.

### arb_duplicate

Declares a temporary arb_int_t, allocates memory for 'data' struct, allocates memory for the bytes using the value in num_bytes, fills the digits, assigns num_bytes/sign, and returns the arb_int_t.

## Arithmetic Operations

I took special care to address every detail about adding and subtracting positive and negative numbers. Not surprisingly subtraction had more cases to address. I break the problem into two parts: figuring out the sign of the result, and implementing the operation. When a result is computed, it needs to go into x. I shrink or expand the byte array in 'x->handle' by calling realloc and then fill it with the result.

### arb_add

Figuring out the sign for addition is fairly simple. When the two signs are the same you keep the sign in the result. When the signs are different adding the two numbers is the equivalent of taking their difference and the sign of the result depends on the order of the numbers. So when a negative and a positive number are passed to arb_add I pass those on to arb_subtract in the same order. See the comments in arb_int.c for details on the operation. As I add digits I store the result one-by-one in an array from left to right. I switch this back to big endian when I fill the result.

### arb_subtract

First you have to check for equivalent arb_ints. If the two arb_ints are the same, then their difference is zero and I immediately fill the result and return. Similar to addition if the signs of the two inputs are different then the operation is equivalent to adding and the sign depends on the first number x. So I switch the sign of y and pass them to arb_add. If the signs are the same it is not as simple as addition to figure out the resulting sign. If the signs are the same then you are taking the difference however, depending on the relative magnitude of the numbers the sign and which number needs to be subtracted from what is different. There are four cases (2 signs, 2 orders). See the code comments for details.

### arb_multiply

Figuring out the sign for multiplication is easy. If the two signs are the same then the result is positive. If they are different then the result is negative. See the code comments for details on my implementation of the multiply operation.

### arb_compare

If the two arb_ints have different signs then you only need to compare the 'sign' member (negative is less than). If both signs are positive then the larger of the two absolute value comparisons is larger. However, if they are both negative then the lesser of the two absolute value comparisons id larger. First I check for the number of digits by comparing the 'num_bytes' member. If they have the same number of digits I run through the units places starting with the most significant digits.

Erik Schluter,
November 10, 2014
