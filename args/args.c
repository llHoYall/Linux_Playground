/*******************************************************************************
 *	@brief		Command line argument
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.01.18	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	printf("Hello World!\n");
	for (int i = 0; i < argc; ++i) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}

	return 0;
}

