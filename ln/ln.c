/*******************************************************************************
 *	@brief		fake ln program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.02.05	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	if (argc < 3) {
		fprintf(stderr, "%s: wrong arguments\n", argv[0]);
		exit(1);
	}

	int c;
	while ((c = getopt(argc, argv, "s")) != -1) {
		switch(c) {
			case 's':
				if (symlink(argv[2], argv[3]) < 0) {
					perror(argv[1]);
					exit(1);
				}
				break;

			case '?':
				fprintf(stderr, "Usage: %s [-s] <source> <destination>\n", argv[0]);
				exit(1);
		}
	}

	if (optind == 1) {
		if (link(argv[1], argv[2]) < 0) {
			perror(argv[1]);
			exit(1);
		}
	}

	exit(0);
}

