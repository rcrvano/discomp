#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	long int min=1;
	long int max=100;
	if ( argc > 2 ) {
		min = atoi(argv[1]);
		if ( min < 1 ) { min = 1; };
		max = atoi(argv[2]);
	};
	printf( "Searching prime numbers in the %ld .. %ld \r\n", min, max );

	for (long int cur_prime=min; cur_prime<max; cur_prime++) {
		bool found_flag = false;
		for (long int i=2; i<cur_prime; i++) {
			if ( cur_prime % i == 0 ) {
				found_flag = true;
			}
		};
		if ( !found_flag && argc < 4 ) {
			printf( "%ld\r\n", cur_prime );
		};
	};
};
