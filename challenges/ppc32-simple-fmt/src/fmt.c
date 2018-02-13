#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

//#define DEBUG
#define STDIN  0
#define STDOUT 1

// Why so global?
static char sbuf1[1024];
static char sbuf2[1024];

static
void (*x_printf)() = (void*) &printf;  // Who doesn't like a lookup in IDA?

static
const char *to_env_name(const char *key)
{
   static char env_name[1024];
   
   memset(env_name, 0, sizeof(env_name));
   snprintf(env_name, sizeof(env_name) - 1, "XP_%s", key);

   return env_name;
}

static
void do_set()
{
   char *key = (char*) sbuf1;
   char *val = (char*) sbuf2;

   //printf("Setting %s=%s\n", key, val);
   setenv(to_env_name(key), val, 1);
}

static
void do_get()
{
   char *key = (char*) sbuf1;
   char *val = getenv(to_env_name(sbuf1));

#ifdef DEBUG
   int stack_var;
   char *p = getenv("PWNFLAG");

   printf("Stack var: 0x%016" PRIxPTR " Env: 0x%016" PRIxPTR "\n", 
      (uintptr_t) &stack_var, (uintptr_t) p);
#endif

   if(val == NULL)
      val = "NOT-FOUND";

   printf("--- %s ---\n", sbuf1);
   x_printf(val);   // Easy peasie format string
   printf("\n--- END ---\n");
}

static
int dispatch(const char *cmd)
{

   if(!strncmp(cmd, "exit", 4)) {
      return 0;
   }
   else if(!strncmp(cmd, "set", 3)) {
      memset(sbuf1, 0, sizeof(sbuf1));
      memset(sbuf2, 0, sizeof(sbuf2));

      if(sscanf(cmd, "set %s %s", sbuf1, sbuf2) == 2)   // Overflow here
         do_set();
   }
   else if(!strncmp(cmd, "get", 3)) {
      memset(sbuf1, 0, sizeof(sbuf1));
      memset(sbuf2, 0, sizeof(sbuf2));

      if(sscanf(cmd, "get %s", sbuf1) > 0)
         do_get();
   }

   return 1;
}

static
void loop_exec_command()
{
   char cmd[128];
   int ret = 0;

   setvbuf(stdout, NULL, _IONBF, 0);
   do {
      memset(cmd, 0, sizeof(cmd));
      write(STDOUT, "> ", 2);
      read(STDIN, cmd, sizeof(cmd) - 1);

      ret = dispatch(cmd);
   } while(ret);
}

int main(int argc, char **argv)
{
   printf("Hello guest!\n");
   loop_exec_command();

   return 0;
}
