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
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <signal.h>

/* Definitions ---------------------------------------------------------------*/
#define SERVER_NAME 						"HoYaHTTP"
#define SERVER_VERSION 					"v1.0"
#define HTTP_MINOR_VERSION			0
#define BLOCK_BUF_SIZE					1024
#define LINE_BUF_SIZE						4096
#define MAX_REQUEST_BODY_LENGTH	(1024 * 1024)
#define TIME_BUF_SIZE						64

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
typedef void (*sighandler_t)(int);

static void Signal_SetHandlers(void);
static void Signal_Get(int sig, sighandler_t handler);
static void Signal_Exit(int sig);

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

static void* hmalloc(size_t sz);
static void Upcase(char* str);
static void Log_PrintAndExit(char* fmt, ...);

/* Main Routines -------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <docroot>\n", argv[0]);
		exit(1);
	}

	Signal_SetHandlers();
	Service(stdin, stdout, argv[1]);
	
	exit(0);
}

/* Private Functions ---------------------------------------------------------*/
static void Signal_SetHandlers(void) {
	Signal_Get(SIGPIPE, Signal_Exit);
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

static void Signal_Exit(int sig) {
	Log_PrintAndExit("exit by signal %d", sig);
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

static void Log_PrintAndExit(char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);

	va_end(ap);
	exit(1);
}

