#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>

//#define DEBUG
#ifdef DEBUG
#define dprintf      printf
#else
#define dprintf
#endif

#define CMD_SET	0x01
#define CMD_GET	0x02
#define CMD_EXIT	0xff

#define FD_STDIN		0
#define FD_STDOUT		1

#define MAX_RANGE		100

struct cache_item
{
	void *p;
	int len;
};

static
struct cache_item *g_cache[MAX_RANGE + 1];

static
int cmd_get()
{
	int i = 0, len = 0;

	read(FD_STDIN, &i, 4);
	read(FD_STDIN, &len, 4);

	dprintf("CACHE GET: i=%d len=%d\n", i, len);

	if(g_cache[i] != NULL)
		write(FD_STDOUT, g_cache[i]->p, len);

	return 0;
}

static
int cmd_set()
{
	int i = 0, len = 0;
	struct cache_item *ci;

	read(FD_STDIN, &i, 4);
	read(FD_STDIN, &len, 4);

	//if(i > ((int) MAX_RANGE))
	//	return 1;

	ci = (void*) malloc(sizeof(*ci) + 1);
	if(NULL == ci)
		return 1;

	ci->p = (void*) malloc(len + 1);
	if(NULL == ci->p)
		return 1;

	read(FD_STDIN, ci->p, len);
	
	if(g_cache[i] != NULL) {
		free(g_cache[i]->p);
		free(g_cache[i]);
	}

	g_cache[i] = ci;
	return 0;
}

static
int do_cmd(int cmd)
{
	int ret = 0;

	dprintf("CMD: %d\n", cmd);

	switch(cmd) {
		case CMD_SET:
			ret = cmd_set();
			break;
		case CMD_GET:
			ret = cmd_get();
			break;
		case CMD_EXIT:
			ret = 1;
			break;
	}

	return ret;
}

static
int srv_loop()
{
	int i;
	int cmd;

	for(i = 0; i < 25; i++) {
		cmd = 0;
		if(read(FD_STDIN, &cmd, 1) != 1)
			break;

		if(do_cmd(cmd))
			break;
	}

   return 0;
}

int main(int argc, char **argv)
{
	return srv_loop();
}
