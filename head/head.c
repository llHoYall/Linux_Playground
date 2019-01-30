/*******************************************************************************
 *	@brief		fake head program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.01.29	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

// for getopt_long
#define _GNU_SOURCE
#include <getopt.h>

/* Definitions ---------------------------------------------------------------*/
#define DEFAULT_N_LINES		10

/* Private Variables ---------------------------------------------------------*/
static struct option longopts[] = {
	{"lines", required_argument,	NULL,	'n'},
	{"help",	no_argument,				NULL,	'h'},
	{0,				0,									0,		0}
};

/* Private Function Prototypes -----------------------------------------------*/
static void head(FILE* f, long nlines);

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	long nlines = DEFAULT_N_LINES;

	int opt;
	while ((opt = getopt_long(argc, argv, "n:", longopts, NULL)) != -1) {
		switch (opt) {
			case 'n':
				nlines = atol(optarg);
				break;

			case 'h':
				fprintf(stdout, "Usage: %s [-n LINES] [FILE ...]\n", argv[0]);
				exit(0);
				
			case '?':
				fprintf(stderr, "Usage: %s [-n LINES] [FILE ...]\n", argv[0]);
				exit(1);
		}
	}

	if (optind == argc) {
		head(stdin, nlines);
	} else {
		for (int i = optind; i < argc; ++i) {
			FILE* f = fopen(argv[i], "r");
			if (!f) {
				perror(argv[i]);
				exit(1);
			}
			head(f, nlines);
			fclose(f);
		}
	}

	exit(0);
}

/* Private Functions ---------------------------------------------------------*/
static void head(FILE* f, long nlines) {
	if (nlines <= 0) {
		return ;
	}
	
	int c;
	while ((c = getc(f)) != EOF) {
		if (putchar(c) < 0) {
			exit(1);
		}
		if (c == '\n') {
			--nlines;
			if (nlines == 0) {
				return ;
			}
		}
	}
}

