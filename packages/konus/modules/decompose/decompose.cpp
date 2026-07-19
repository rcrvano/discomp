#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

int main(int argc, char** argv)
{
	long int max=100;
	int count = 1;
	if ( argc > 2 ) {
		max = atoi(argv[1]);
		count = atoi(argv[2]);
	} else {
		printf( "Usage: %s <max_value> <block_count>", argv[0] );
		return -1;
	};
	if ( count < 1 ) { count = 1; }

	int portion_size = max/count;
	for ( int i = 1; i <= count; i++ ) {
		ofstream myfile;
		char buffer [33];
		sprintf (buffer, "input%d.txt", i);
		myfile.open (buffer);
		myfile << portion_size*(i-1) + 1 << " " << portion_size*i;
		myfile.close();
	};
	return 0;
};

