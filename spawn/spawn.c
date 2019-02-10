/*******************************************************************************
 *	@brief		fake spawn program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.02.10	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <command> <arg>\n", argv[0]);
		exit(1);
	}

	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "fork(2) failed\n");
		exit(1);
	}
	// Child process
	if (pid == 0) {
		execl(argv[1], argv[1], argv[2], NULL);
		perror(argv[1]);
		exit(99);
	} 
	// Parent process
	else {
		int status;
		waitpid(pid, &status, 0);
		printf("child (PID=%d) finished; ", pid);
		if (WIFEXITED(status)) {
			printf("exit, status=%d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("signal, sig=%d\n", WTERMSIG(status));
		} else {
			printf("abnormal exit\n");
		}
	}

	exit(0);
}

