/*******************************************************************************
 *	@brief		daytime protocol
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.01.18	Created.
 *******************************************************************************
 *		The Daytime Protocol is a service in the Internet Protocol Suite, defined
 *	in 1983 in RFC 867. A host may connect to a server that supports the Daytime
 *	protocol on either TCP or UDP port 13. The server returns an ASCII character
 *	string of the current date and time in an unspecified format.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

/* Private Function Prototypes -----------------------------------------------*/
static int OpenConnection(const char* const host, const char* const service);

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	int sock = OpenConnection((argc > 1 ? argv[1] : "localhost"), "daytime");
	FILE* f = fdopen(sock, "r");
	if (!f) {
		perror("fdopen(3)");
		exit(1);
	}

	char buf[1024];
	fgets(buf, sizeof buf, f);
	fclose(f);
	fputs(buf, stdout);
	exit(0);
}

/* Private Functions ---------------------------------------------------------*/
static int OpenConnection(const char* const host, const char* const service) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo* res;
	int err;
	if ((err = getaddrinfo(host, service, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo(3): %s\n", gai_strerror(err));
		exit(1);
	}

	struct addrinfo* ai;
	for (ai = res; ai; ai = ai->ai_next) {
		int sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock < 0) {
			continue;
		}
		
		if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
			close(sock);
			continue;
		}

		freeaddrinfo(res);
		return sock;
	}

	fprintf(stderr, "socket(2) / connect(2) failed");
	freeaddrinfo(res);
	exit(1);
}

