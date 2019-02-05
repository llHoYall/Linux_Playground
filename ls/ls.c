/*******************************************************************************
 *	@brief		fake ls program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.02.05	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

/* Private Function Prototypes -----------------------------------------------*/
static void ls(char* path);

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "%s: no arguments\n", argv[0]);
		exit(1);
	}

	for (int i = 1; i < argc; ++i) {
		ls(argv[i]);
	}

	exit(0);
}

/* Private Functions ---------------------------------------------------------*/
static void ls(char* path) {
	DIR* d = opendir(path);
	if (!d) {
		perror(path);
		exit(1);
	}

	struct dirent* ent;
	while (ent = readdir(d)) {
		printf("%s\n", ent->d_name);
	}
	closedir(d);
}

