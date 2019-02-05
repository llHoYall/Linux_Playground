/*******************************************************************************
 *	@brief		fake mkdir program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.02.05	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "%s: no arguments\n", argv[0]);
		exit(1);
	}

	for (int i = 1; i < argc; ++i) {
		if (mkdir(argv[i], 0777) < 0) {
			perror(argv[i]);
			exit(1);
		}
	}

	exit(0);
}

