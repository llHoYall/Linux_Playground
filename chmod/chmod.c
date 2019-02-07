/*******************************************************************************
 *	@brief		fake chmod program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.02.07	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "no mode given\n");
		exit(1);
	}

	int mode = strtol(argv[1], NULL, 8);
	for (int i = 2; i < argc; ++i) {
		if (chmod(argv[i], mode) < 0) {
			perror(argv[i]);
		}
	}

	exit(0);
}

