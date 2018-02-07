// apt-get install libseccomp-dev
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <seccomp.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

//#define DEBUG
#ifdef DEBUG
#define dprintf		printf
#else
#define dprintf
#endif

#define SC_MAX_SIZE		(1024 * 1024)
#define URL_MAX_SIZE		4096

void sa(int s)
{
	dprintf("Signal quit! pid:%d sig:%d\n", getpid(), s);
	exit(1);
}

static
void http_fetch(char *host, unsigned short port, char *path)
{
	__asm__("nop");
}

static
void cmd_do_http_fetch(char *resource)
{
	char host[512];
	unsigned short port = 80;
	char *path = "/";
	char *ptr;
	char *end_str;

	dprintf("Host is @ %p\n", &host);

	if(strncmp(resource, "http://", 7) != 0) {
		dprintf("Invalid URL\n");
		return;
	}

	ptr = resource;
	ptr += 7;

	end_str = resource + strlen(resource);
	while(ptr > end_str)
	{
		if(*ptr == ':') {
			*ptr++ = '\0';
			port = atoi(ptr);
		}
		else if(*ptr == '/') {
			*ptr++ = '\0';
			path = ptr;
			break;
		}

		ptr++;
	}

	ptr = resource + strlen("http://");
	strncpy(host, ptr, strlen(ptr));
	//dprintf("Fetching: Host:%s Port:%d Path:%s\n", host, port, path);
	http_fetch(host, port, path);
}

static
void cmd_http_fetch(char cmd, int fd)
{
	char *url;
	size_t s;
	int ret, n;

	s = 0;
	ret = read(fd, (void*) &s, 4);
	assert(ret > 0);

	if(s > URL_MAX_SIZE)
		return;

	url = (char*) calloc(s + 1, 1);
	assert(url != NULL);

	//dprintf("URL size: %d\n", s);

	n = 0;
	while(s) {
		ret = read(fd, url + n, s);
		if(ret < 0)
			break;

		//dprintf("Read %d bytes\n", ret);

		s -= ret;
		n += ret;
	}

	if(s != 0)
		return;

	cmd_do_http_fetch(url);
}

static
void cmd_unimplemented(char cmd, int fd)
{
	dprintf("Unimplemented command: 0x%02x\n", (unsigned int) cmd);
}

static
void handle_cmd(int fd, char cmd)
{
	dprintf("Handling cmd: 0x%02x\n", (unsigned int) cmd);

	switch(cmd)
	{
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
			cmd_unimplemented(cmd, fd);
			break;
		case 0x0e:
			cmd_http_fetch(cmd, fd);
			break;
		default:
			dprintf("Unknown command 0x%02x\n", (unsigned int) cmd);
			break;
	}
}

static
void handler_loop(int fd)
{
	int i = 0;
	int ret;
	char cmd;

	while(++i) {
		//sleep(1);
		dprintf("Running handler loop: %d [fd=%d]\n", i, fd);
		ret = read(fd, &cmd, 1);
		if(ret <= 0)
			break;

		handle_cmd(fd, cmd);
	}
}

int main(int argc, char **argv)
{
	scmp_filter_ctx		sctx;
	int						ret, n, s;
	void						(*f)();
	void						*psc;
	int						pipe_fds[2];

	// standalone test
	//handler_loop(STDIN_FILENO);
	//exit(0);

	dprintf("//----------------- Good Morning!! \n");

	ret = pipe(pipe_fds);	// [0]:read [1]:write
	assert(ret == 0);

	dprintf("Pipe created %d/%d\n", pipe_fds[0], pipe_fds[1]);
	
	if(!fork()) {
		dprintf("Forked handler process with pid: %d\n", getpid());

		signal(SIGALRM, sa);
		alarm(5);
		
		handler_loop(pipe_fds[0]);
		exit(0);
	}
	
	signal(SIGCHLD, SIG_IGN);
	signal(SIGSEGV, sa);
	signal(SIGTRAP, sa);
	signal(SIGALRM, sa);
	alarm(5);

	s = 0;
	if(read(STDIN_FILENO, (void*) &s, 4) <= 0)
		goto error_out;

	psc = mmap(NULL, (size_t) s, PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(psc != NULL);
	
	dprintf("Allocated %d bytes of stage1 memory at %p\n", s, psc);

	n = 0;
	while(s) {
		ret = read(STDIN_FILENO, ((char*)psc) + n, s);
		if(ret < 0)
			break;
		n += ret;
		s -= ret;
	}

	if(s)
		goto error_out;

	sctx = seccomp_init(SCMP_ACT_KILL);
	if(sctx == NULL) goto error_out;

	//ret = seccomp_rule_add(sctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0);
	//if(ret < 0) goto error_out;
	ret = seccomp_rule_add(sctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
	if(ret < 0) goto error_out;
	ret = seccomp_rule_add(sctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
	if(ret < 0) goto error_out;
	ret = seccomp_rule_add(sctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
	if(ret < 0) goto error_out;
	ret = seccomp_rule_add(sctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
	if(ret < 0) goto error_out;

	dprintf("Loading sandbox rules\n");
	ret = seccomp_load(sctx);
	if(ret < 0) goto error_out;

	dprintf("Sandbox active!!\n");
	dprintf("Running stage1\n");
	f = (void*) psc;
	f();

error_out:
	perror("error");
	return 0;
}
