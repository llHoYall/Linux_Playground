/*******************************************************************************
 *	@brief		fake mv program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.02.06	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "%s: wrong arguments\n", argv[0]);
		exit(1);
	}

	if (rename(argv[1], argv[2]) < 0) {
		perror(argv[1]);
		exit(1);
	}

	exit(0);
}

