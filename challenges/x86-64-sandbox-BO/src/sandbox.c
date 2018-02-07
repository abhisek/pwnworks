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
#define dprintf      printf
#else
#define dprintf
#endif

#define SC_MAX_SIZE     (1024 * 1024)

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
void cmd_unimplemented(char cmd, int fd)
{
   dprintf("Unimplemented command: 0x%02x\n", (unsigned int) cmd);
}

static
void handle_pwnable(int fd)
{
   char s_buf[100];
   int n;

   n = 0;
   if(read(fd, (char*) &n, 4) <= 0)
      return;

   dprintf("Running pwnable, copying %d bytes\n", n);

   memset(s_buf, 0, sizeof(s_buf));
   read(fd, s_buf, n);  // XXX: Too easy, need to make it tricky!
}

static
void handle_cmd(int fd, char cmd)
{
   dprintf("Handling cmd: 0x%02x\n", (unsigned int) cmd);

   switch(cmd)
   {
      case 0x01:
         cmd_unimplemented(cmd, fd);
      case 0x02:
         cmd_unimplemented(cmd, fd);
      case 0x03:
         cmd_unimplemented(cmd, fd);
      case 0x04:
         cmd_unimplemented(cmd, fd);
      case 0x05:
         cmd_unimplemented(cmd, fd);
         break;
      case 0x0a:
         handle_pwnable(fd);
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
   scmp_filter_ctx      sctx;
   int                  ret, n, s;
   void                 (*f)();
   void                 *psc;
   int                  pipe_fds[2];

   // standalone test
   //handler_loop(STDIN_FILENO);
   //exit(0);

   dprintf("//----------------- Good Morning!! \n");

   ret = pipe(pipe_fds);   // [0]:read [1]:write
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

   if(s > SC_MAX_SIZE) {
      dprintf("Requested size too large\n");
      goto error_out;
   }

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
   //ret = seccomp_rule_add(sctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
   //if(ret < 0) goto error_out;

   // To exploit, we just need to write to pipe
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
