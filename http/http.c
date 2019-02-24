/*******************************************************************************
 *	@brief		fake http server program.
 *	@author		llHoYall <hoya128@gmail.com>
 *	@version	v1.0.0
 *	@history
 *		2019.02.16	Created.
 ******************************************************************************/

/* Include Headers -----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>

#define _GNU_SOURCE
#include <getopt.h>

/* Definitions ---------------------------------------------------------------*/
#define SERVER_NAME 						"HoYaHTTP"
#define SERVER_VERSION 					"v1.0"
#define HTTP_MINOR_VERSION			0
#define BLOCK_BUF_SIZE					1024
#define LINE_BUF_SIZE						4096
#define MAX_REQUEST_BODY_LENGTH	(1024 * 1024)
#define TIME_BUF_SIZE						64
#define MAX_BACKLOG							5
#define DEFAULT_PORT						"80"

#define USAGE	"Usage: %s [--port=n] [--chroot --user=u --group=g] [--debug] <docroot>\n"

/* Private Variables ---------------------------------------------------------*/
static int debug_mode = 0;
static struct option longopts[] = {
	{"debug", 	no_argument, 				&debug_mode, 	1},
	{"chroot", 	no_argument, 				NULL, 				'c'},
	{"user", 		required_argument, 	NULL, 				'u'},
	{"group", 	required_argument, 	NULL, 				'g'},
	{"port", 		required_argument, 	NULL, 				'p'},
	{"help", 		no_argument, 				NULL, 				'h'},
	{0, 				0, 									0, 						0}
};

/* Structures ----------------------------------------------------------------*/
struct HTTPHeaderField {
	char* name;
	char* value;
	struct HTTPHeaderField* next;
};

struct HTTPRequest {
	int protocol_minor_version;
	char* method;
	char* path;
	struct HTTPHeaderField* header;
	char* body;
	long length;
};

struct FileInfo {
	char* path;
	long size;
	int ok;
};

/* Private Function Prototypes -----------------------------------------------*/
static void SetupEnvironment(char* root, char* user, char* group);

typedef void (*sighandler_t)(int);

static void Signal_SetHandlers(void);
static void Signal_Get(int sig, sighandler_t handler);
static void Signal_DetachChildren(void);
static void Signal_Exit(int sig);

static void Handler_Noop(int sig);

static void Service(FILE* in, FILE* out, char* docroot);
static struct HTTPRequest* HTTP_ReadRequest(FILE* in);
static void HTTP_ReadRequestLine(struct HTTPRequest* req, FILE* in);
static struct HTTPHeaderField* HTTP_ReadHeaderField(FILE* in);
static void HTTP_FreeRequest(struct HTTPRequest* req);
static long HTTP_GetContentLength(struct HTTPRequest* req);
static char* HTTP_LookupHeaderFieldValue(struct HTTPRequest* req, char* name);
static void HTTP_RespondTo(struct HTTPRequest* req, FILE* out, char* docroot);
static void HTTP_DoFileResponse(struct HTTPRequest* req, FILE* out, char* docroot);
static void HTTP_MethodNotAllowed(struct HTTPRequest* req, FILE* out);
static void HTTP_NotImplemented(struct HTTPRequest* req, FILE* out);
static void HTTP_NotFound(struct HTTPRequest* req, FILE* out);
static void HTTP_OutputCommonHeaderFields(struct HTTPRequest* req, FILE* out, char* status);
static struct FileInfo* HTTP_GetFileInfo(char* docroot, char* urlpath);
static char* HTTP_BuildFsPath(char* docroot, char* urlpath);
static void HTTP_FreeFileInfo(struct FileInfo* info);
static char* HTTP_GuessContentType(struct FileInfo* info);
static int HTTP_ListenSocket(char* port);
static void HTTP_ServerMain(int server_fd, char* docroot);

static void BecomeDaemon(void);
static void* hmalloc(size_t sz);
static void Upcase(char* str);
static void Log_PrintAndExit(const char* fmt, ...);

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	int do_chroot = 0;
	char* user = NULL;
	char* group = NULL;
	char* port = NULL;
	int opt;
	while ((opt = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
		switch (opt) {
			case 0:
				break;

			case 'c':
				do_chroot = 1;
				break;

			case 'u':
				user = optarg;
				break;

			case 'g':
				group = optarg;
				break;

			case 'p':
				port = optarg;
				break;

			case 'h':
				fprintf(stdout, USAGE, argv[0]);
				exit(0);

			case '?':
				fprintf(stderr, USAGE, argv[0]);
				exit(1);
		}
	}
	if (optind != argc - 1) {
		fprintf(stderr, USAGE, argv[0]);
		exit(1);
	}
	char* docroot = argv[optind];

	if (do_chroot) {
		SetupEnvironment(docroot, user, group);
		docroot = "";
	}

	Signal_SetHandlers();
	int server_fd = HTTP_ListenSocket(port);
	if (!debug_mode) {
		openlog(SERVER_NAME, LOG_PID | LOG_NDELAY, LOG_DAEMON);
		BecomeDaemon();
	}
	HTTP_ServerMain(server_fd, docroot);
	
	exit(0);
}

/* Private Functions ---------------------------------------------------------*/
static void SetupEnvironment(char* root, char* user, char* group) {
	if (!user || !group) {
		fprintf(stderr, "use both of --user and --group\n");
		exit(1);
	}

	struct group* gr = getgrnam(group);
	if (!gr) {
		fprintf(stderr, "no such group: %s\n", group);
		exit(1);
	}
	if (setgid(gr->gr_gid) < 0) {
		perror("setgid(2)");
		exit(1);
	}
	if (initgroups(user, gr->gr_gid) < 0) {
		perror("initgroup(2)");
		exit(1);
	}
	
	struct passwd* pw = getpwnam(user);
	if (!pw) {
		fprintf(stderr, "no such user: %s\n", user);
		exit(1);
	}

	chroot(root);
	if (setuid(pw->pw_uid) < 0) {
		perror("setuid(2)");
		exit(1);
	}
}

static void Signal_SetHandlers(void) {
	Signal_Get(SIGTERM, Signal_Exit);
	Signal_DetachChildren();
}

static void Signal_Get(int sig, sighandler_t handler) {
	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(sig, &act, NULL) < 0) {
		Log_PrintAndExit("sigaction() failed: %s", strerror(errno));
	}
}

static void Signal_DetachChildren(void) {
	struct sigaction act;
	act.sa_handler = Handler_Noop;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
	if (sigaction(SIGCHLD, &act, NULL) < 0) {
		Log_PrintAndExit("sigaction() failed: %s", strerror(errno));
	}
}

static void Signal_Exit(int sig) {
	Log_PrintAndExit("exit by signal %d", sig);
}

static void Handler_Noop(int sig) {
	(void)sig;
}

static void Service(FILE* in, FILE* out, char* docroot) {
	struct HTTPRequest* req = HTTP_ReadRequest(in);
	HTTP_RespondTo(req, out, docroot);
	HTTP_FreeRequest(req);
}

static struct HTTPRequest* HTTP_ReadRequest(FILE* in) {
	struct HTTPRequest* req = hmalloc(sizeof(struct HTTPRequest));
	HTTP_ReadRequestLine(req, in);
	req->header = NULL;

	struct HTTPHeaderField *h;
	while ((h = HTTP_ReadHeaderField(in))) {
		h->next = req->header;
		req->header = h;
	}
	req->length = HTTP_GetContentLength(req);
	if (req->length != 0) {
		if (req->length > MAX_REQUEST_BODY_LENGTH) {
			Log_PrintAndExit("request body too long");
		}
		req->body = hmalloc(req->length);
		if (fread(req->body, req->length, 1, in) < 1) {
			Log_PrintAndExit("failed to read request body");
		}
	} else {
		req->body = NULL;
	}

	return req;
}

static void HTTP_ReadRequestLine(struct HTTPRequest* req, FILE* in) {
	char buf[LINE_BUF_SIZE];
	if (!fgets(buf, LINE_BUF_SIZE, in)) {
		Log_PrintAndExit("no request line");
	}

	char* p = strchr(buf, ' ');
	if (!p) {
		Log_PrintAndExit("parse error on request line (1): %s", buf);
	}
	*p++ = '\0';

	req->method = hmalloc(p - buf);
	strcpy(req->method, buf);
	Upcase(req->method);

	char* path = p;
	p = strchr(path, ' ');
	if (!p) {
		Log_PrintAndExit("parse error on request line (2): %s", buf);
	}
	*p++ = '\0';
	req->path = hmalloc(p - path);
	strcpy(req->path, path);
	if (strncasecmp(p, "HTTP/1.", strlen("HTTP/1.")) != 0) {
		Log_PrintAndExit("parse error on request line (3): %s", buf);
	}
	p += strlen("HTTP/1.");
	req->protocol_minor_version = atoi(p);
}

static struct HTTPHeaderField* HTTP_ReadHeaderField(FILE* in) {
	char buf[LINE_BUF_SIZE];
	if (!fgets(buf, LINE_BUF_SIZE, in)) {
		Log_PrintAndExit("failed to read request header field: %s", strerror(errno));
	}
	if ((buf[0] == '\n') || (strcmp(buf, "\r\n") == 0)) {
		return NULL;
	}

	char* p = strchr(buf, ':');
	if (!p) {
		Log_PrintAndExit("parse error on request header field: %s", buf);
	}
	*p++ = '\0';

	struct HTTPHeaderField* h = hmalloc(sizeof(struct HTTPHeaderField));
	h->name = hmalloc(p - buf);
	strcpy(h->name, buf);
	p += strspn(p, " \t");
	h->value = hmalloc(strlen(p) + 1);
	strcpy(h->value, p);
	return h;
}

static void HTTP_FreeRequest(struct HTTPRequest* req) {
	struct HTTPHeaderField* head = req->header;
	while (head) {
		struct HTTPHeaderField* h = head;
		head = head->next;
		free(h->name);
		free(h->value);
		free(h);
	}
	free(req->method);
	free(req->path);
	free(req->body);
	free(req);
}

static long HTTP_GetContentLength(struct HTTPRequest* req) {
	char* val = HTTP_LookupHeaderFieldValue(req, "Content-Length");
	if (!val) {
		return 0;
	}

	long len = atoi(val);
	if (len < 0) {
		Log_PrintAndExit("negative Content-Length value");
	}
	return len;
}

static char* HTTP_LookupHeaderFieldValue(struct HTTPRequest* req, char* name) {
	struct HTTPHeaderField* h;
	for (h = req->header; h; h = h->next) {
		if (strcasecmp(h->name, name) == 0) {
			return h->value;
		}
	}
	return NULL;
}

static void HTTP_RespondTo(struct HTTPRequest* req, FILE* out, char* docroot) {
	if (strcmp(req->method, "GET") == 0) {
		HTTP_DoFileResponse(req, out, docroot);
	} else if (strcmp(req->method, "HEAD") == 0) {
		HTTP_DoFileResponse(req, out, docroot);
	} else if (strcmp(req->method, "POST") == 0) {
		HTTP_MethodNotAllowed(req, out);
	} else {
		HTTP_NotImplemented(req, out);
	}
}

static void HTTP_DoFileResponse(struct HTTPRequest* req, FILE* out, char* docroot) {
	struct FileInfo* info = HTTP_GetFileInfo(docroot, req->path);
	if (!info->ok) {
		HTTP_FreeFileInfo(info);
		HTTP_NotFound(req, out);
		return ;
	}

	HTTP_OutputCommonHeaderFields(req, out, "200 OK");
	fprintf(out, "Content-Length: %ld\r\n", info->size);
	fprintf(out, "Content-Type: %s\r\n", HTTP_GuessContentType(info));
	fprintf(out, "\r\n");

	if (strcmp(req->method, "HEAD") != 0) {
		int fd = open(info->path, O_RDONLY);
		if (fd < 0) {
			Log_PrintAndExit("failed to open %s: %s", info->path, strerror(errno));
		}

		char buf[BLOCK_BUF_SIZE];
		for (;;) {
			ssize_t n = read(fd, buf, BLOCK_BUF_SIZE);
			if (n < 0) {
				Log_PrintAndExit("failed to read %s: %s", info->path, strerror(errno));
			}
			if (n == 0) {
				break;
			}
			if (fwrite(buf, 1, n, out) < n) {
				Log_PrintAndExit("failed to write to socket: %s", strerror(errno));
			}
		}
		close(fd);
	}
	fflush(out);
	HTTP_FreeFileInfo(info);
}

static void HTTP_MethodNotAllowed(struct HTTPRequest* req, FILE* out) {
	HTTP_OutputCommonHeaderFields(req, out, "405 Method Not Allowed");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	fprintf(out, "<html>\r\n");
	fprintf(out, "<header>\r\n");
	fprintf(out, "<title>405 Method Not Allowed</title>\r\n");
	fprintf(out, "<header>\r\n");
	fprintf(out, "<body>\r\n");
	fprintf(out, "<p>The request method %s is not allowed</p>\r\n", req->method);
	fprintf(out, "</body>\r\n");
	fprintf(out, "</html>\r\n");
	fflush(out);
}

static void HTTP_NotImplemented(struct HTTPRequest* req, FILE* out) {
	HTTP_OutputCommonHeaderFields(req, out, "501 Not Implemented");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	fprintf(out, "<html>\r\n");
	fprintf(out, "<header>\r\n");
	fprintf(out, "<title>501 Not Implemented</title>\r\n");
	fprintf(out, "<header>\r\n");
	fprintf(out, "<body>\r\n");
	fprintf(out, "<p>The request method %s is not implemented</p>\r\n", req->method);
	fprintf(out, "</body>\r\n");
	fprintf(out, "</html>\r\n");
	fflush(out);
}

static void HTTP_NotFound(struct HTTPRequest* req, FILE* out) {
	HTTP_OutputCommonHeaderFields(req, out, "404 Not Found");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	if (strcmp(req->method, "HEAD") != 0) {
		fprintf(out, "<html>\r\n");
		fprintf(out, "<header><title>Not Found</title></header>\r\n");
		fprintf(out, "<body><p>File not found</p></body>\r\n");
		fprintf(out, "</html>\r\n");
	}
	fflush(out);
}

static void HTTP_OutputCommonHeaderFields(struct HTTPRequest* req, FILE* out, char* status) {
	time_t t = time(NULL);
	struct tm* tm = gmtime(&t);
	if (!tm) {
		Log_PrintAndExit("gmtime() failed: %s", strerror(errno));
	}

	char buf[TIME_BUF_SIZE];
	strftime(buf, TIME_BUF_SIZE, "%a, %d %b %Y %H:%M:%S GMT", tm);
	fprintf(out, "HTTP/1.%d %s\r\n", HTTP_MINOR_VERSION, status);
	fprintf(out, "Date: %s\r\n", buf);
	fprintf(out, "Server: %s/%s\r\n", SERVER_NAME, SERVER_VERSION);
	fprintf(out, "Connection: close\r\n");
}

static struct FileInfo* HTTP_GetFileInfo(char* docroot, char* urlpath) {
	struct FileInfo* info = hmalloc(sizeof(struct FileInfo));
	info-> path = HTTP_BuildFsPath(docroot, urlpath);
	info->ok = 0;
	struct stat st;
	if (lstat(info->path, &st) < 0) {
		return info;
	}
	if (!S_ISREG(st.st_mode)) {
		return info;
	}
	info->ok = 1;
	info->size = st.st_size;
	return info;
}

static char* HTTP_BuildFsPath(char* docroot, char* urlpath) {
	char* path = hmalloc(strlen(docroot) + 1 + strlen(urlpath) + 1);
	sprintf(path, "%s/%s", docroot, urlpath);
	return path;
}

static void HTTP_FreeFileInfo(struct FileInfo* info) {
	free(info->path);
	free(info);
}

static char* HTTP_GuessContentType(struct FileInfo* info) {
	return "text/plain";
}

static int HTTP_ListenSocket(char* port) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int err;
	struct addrinfo* res;
	if ((err = getaddrinfo(NULL, port, &hints, &res)) != 0) {
		Log_PrintAndExit(gai_strerror(err));
	}

	struct addrinfo* ai;
	for (ai = res; ai; ai = ai->ai_next) {
		int sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock < 0) {
			continue;
		}
		if (bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
			close(sock);
			continue;
		}
		freeaddrinfo(res);
		return sock;
	}
	Log_PrintAndExit("failed to listen socket");
	return -1;
}

static void HTTP_ServerMain(int server_fd, char* docroot) {
	for (;;) {
		struct sockaddr_storage addr;
		socklen_t addrlen = sizeof addr;
		int sock = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
		if (sock < 0) {
			Log_PrintAndExit("accept(2) failed: %s", strerror(errno));
		}
		int pid = fork();
		if (pid < 0) {
			exit(3);
		}
		if (pid == 0) {
			FILE* inf = fdopen(sock, "r");
			FILE* outf = fdopen(sock, "w");
			Service(inf, outf, docroot);
			exit(0);
		}
		close(sock);
	}
}

static void BecomeDaemon(void) {
	if (chdir("/") < 0) {
		Log_PrintAndExit("chdir(2) failed: %s", strerror(errno));
	}
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);

	int n = fork();
	if (n < 0) {
		Log_PrintAndExit("fork(2) failed: %s", strerror(errno));
	}
	if (n != 0) {
		_exit(0);
	}
	if (setsid() < 0) {
		Log_PrintAndExit("setsid(2) failed: %s", strerror(errno));
	}
}

static void* hmalloc(size_t sz) {
	void* p = malloc(sz);
	if (!p) {
		Log_PrintAndExit("failed to allocate memory");
	}
	return p;
}

static void Upcase(char* str) {
	char* p;
	for (p = str; *p; ++p) {
		*p = (char)toupper((int)*p);
	}
}

static void Log_PrintAndExit(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	if (debug_mode) {
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
	} else {
		vsyslog(LOG_ERR, fmt, ap);
	}

	va_end(ap);
	exit(1);
}

