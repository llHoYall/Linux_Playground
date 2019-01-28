/*******************************************************************************
 *	@brief		fake cat program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.01.27	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Definitions ---------------------------------------------------------------*/
#define BUFFER_SIZE		2048

/* Private Function Prototypes -----------------------------------------------*/
static void cat(const char* path);
static void die(const char* s);

/* Main Routines -------------------------------------------------------------*/
// Syscall
int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "%s: file name not given\n", argv[0]);
		exit(1);
	}

	for (int i = 1; i < argc; ++i) {
		cat(argv[i]);
	}

	exit(0);
}

// stdio
/*
int main (int argc, char* argv[]) {
	for (int i = 1; i < argc; ++i) {
		FILE* f = fopen(argv[i], "r");
		if (!f) {
			perror(argv[i]);
			exit(1);
		}

		int c;
		while ((c = fgetc(f)) != EOF) {
			printf("[DBG] %x\n", c);
			if (putchar(c) < 0) {
				exit(1);
			}
		}

		fclose(f);
	}

	exit(0);
}
*/

/* Private Functions ---------------------------------------------------------*/
static void cat(const char* path) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		die(path);
	}

	for (;;) {
		unsigned char buf[BUFFER_SIZE];
		int n = read(fd, buf, sizeof buf);
		if (n < 0) {
			die(path);
		}
		else if (n == 0) {
			break;
		}
		if (write(STDOUT_FILENO, buf, n) < 0) {
			die(path);
		}
	}
	if (close(fd) < 0) {
		die(path);
	}
}

static void die(const char* s) {
	perror(s);
	exit(1);
}

