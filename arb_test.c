// Erik Schluter
// 11/9/2014

#include <stdio.h>
#include <time.h>
#include "arb_int.h"

#define MAX_OUT_LEN 4000

double pow2 (double base, double exponent) {  // implement power function from math.h so you
	int i;				      // don't have to manually link with -lm
	int ret = 1;
	
	for (i = 0; i < exponent; i++)
		ret *= base;

	return ret;
}   // This is actually ridiculously slow, which is why I use mod prior to passing exponent

int main (int argc, char ** argv) {

	arb_int_t test1, test2;
	char out[MAX_OUT_LEN];

	if ( (argv[1] != NULL) && (argv[2] != NULL) && (argv[3] != NULL) && (argv[4] == NULL) ) {
		
		if ( !arb_from_string(&test1, argv[1]) ) {
			fprintf (stderr, "ERROR: test1 conversion failed\n");
			exit(1);
		}

		if ( !arb_from_string(&test2, argv[3]) ) {
			fprintf (stderr, "ERROR: test2 conversion failed\n");
			arb_free (test1);
			exit(1);
		}

		switch (argv[2][0]) {
			case '+':
				arb_add (test1, test2);
				break;
			case '-':
				arb_subtract (test1, test2);
				break;
			case '*':
				arb_multiply (test1, test2);
				break;
		}

		if ( !arb_to_string( test1, out, MAX_OUT_LEN) )
			fprintf (stderr, "ERROR: could not convert test2 to back to integer\n");

		printf("%s", out);

	} else if (argv[1] == NULL) {
		long outi;
		char str1[11];
		char out[30];
		int rand1, randSign1, randSign2;
		long rand2;
	
		srand (time(NULL));
		rand1     = rand();
		rand2     = rand();
		randSign1 = rand();
		randSign2 = rand();

		// create both random negative and positive numbers
		rand1 *= (int)pow2((double)(-1), (double)(randSign1 % 2));
		rand2 *= (long)pow2((double)(-1), (double)(randSign2 % 2));

		sprintf (str1, "%d", rand1);

		// test arb_from_string
		if ( !arb_from_string (&test1, str1) ) {
			fprintf (stderr, "WARNING: arbX conversion failed\n");
			exit(1);
		}

		// test arb_from_int
		if ( !arb_from_int(&test2, rand2) ) {
			fprintf (stderr, "WARNING: arbY conversion failed\n");
			arb_free (test1);
			exit(1);
		}
		
		// test arb_add
		arb_add (test1, test2);

		// test arb_subtract with result from arb_add
		arb_subtract (test1, test2);

		// test arb_multiply with result from arb_subtract
		arb_multiply (test1, test2);

		// convert test2 back to int but do not print
		if ( !arb_to_int(test2, &outi) ) {
			fprintf (stderr, "ERROR: could not convert test2 to back to integer\n");
		}

		// convert test1 back to string and print
		if ( !arb_to_string(test1, out, 30) ) {
			fprintf (stderr, "ERROR: could not convert test1 back to string\n");
		}
		printf ("%s", out);
	} else {
		fprintf (stderr, "\nNo command line input tests all manipulations on a randomly generated number\n");
		fprintf (stderr, "Specify two numbers and an operation like so: \n");
		fprintf (stderr, "\n	arb_int_1	'operator'	arb_int_2\n\nexiting...\n");
		exit(1);
	}
		

	arb_free (test1);
	arb_free (test2);

	return EXIT_SUCCESS;
}
