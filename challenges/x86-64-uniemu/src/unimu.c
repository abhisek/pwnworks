#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <unicorn/unicorn.h>

#define MAX_BUFFER	10240L
#define FD_STDIN 		0

//#define DEBUG
#ifdef DEBUG
#define dprintf      printf
#else
#define dprintf
#endif

#define ADDRESS 		0x1000000
#define EMU_VM_SIZE	(2 * 1024 * 1024)
#define STACK_OFFSET	0x200000
#define MAX_INTRS		(0xff + 1)

#define X86_SYSTEM_CALL		0x80
#define X86_SYS_EXIT			0x01
#define X86_SYS_READ			0x03
#define X86_SYS_WRITE		0x04
#define X86_SYS_MMAP			0x5a
#define X86_SYS_MUNMAP		0x5b
#define X86_SYS_INITMOD		0x80
#define X86_SYS_FINIMOD		0x81

#define MMAP_MAX_ALLOC		0x200U
#define MAX_MODULES			64

struct emu_module_t
{
	char		name[64];
	void 		*image;
	size_t	len;
	void		(*init)();
	void		(*fini)();
};

struct emu_module_t *module_table[MAX_MODULES];
static unsigned int module_table_idx;

struct emu_mem_t
{
	int 		mem_id;
	void 		*addr;
	size_t	len;

	struct emu_mem_t *next;
};

static
struct emu_mem_t *mem_list_head;

static
int intrs_table[MAX_INTRS];

static
void emu_mod_init()
{
	dprintf("Module Init\n");
}

static
void emu_mod_fini()
{
	dprintf("Module Fini\n");
}

static
void emu_do_exit(int n)
{
	exit(n);
}

static
int mem_list_add(void *addr, size_t len)
{
	struct emu_mem_t *m = (struct emu_mem_t*) malloc(sizeof(*m) + 1);
	if(NULL == m)
		return -1;

	m->addr = addr;
	m->len = len;
	m->next = mem_list_head;

	if(mem_list_head)
		m->mem_id = mem_list_head->mem_id + 1;
	else
		m->mem_id = 1;

	mem_list_head = m;
	return m->mem_id;
}

static
void mem_free(struct emu_mem_t *m)
{
	free(m->addr);
	free(m);
}

static
void mem_list_remove(struct emu_mem_t *m)
{
	struct emu_mem_t *p;

	if(m == mem_list_head) {
		mem_list_head = m->next;
		mem_free(m);
		return;
	}

	p = mem_list_head;
	while(p->next) {
		if(p->next == m) {
			p->next = m->next;
			mem_free(m);
			break;
		}

		p = p->next;
	}
}

static
void emu_mod_release(struct emu_module_t *mod)
{
	if(NULL == mod)
		return;

	mod->fini();
	free(mod);
}

static
void emu_do_init_module(uc_engine *uc, void *addr, size_t len, const char **params)
{
	struct emu_module_t *mod = malloc(sizeof(*mod));
	int ret = -1;

	uc_reg_write(uc, UC_X86_REG_EAX, &ret);
	if(NULL == mod)
		return;

	mod->init = &emu_mod_init;
	mod->fini = &emu_mod_fini;
	sprintf(mod->name, "Module at 0x%08x", (unsigned int) addr);

	ret = module_table_idx;
	if(module_table[module_table_idx] != NULL)
		emu_mod_release(module_table[module_table_idx]);

	module_table[module_table_idx++] = mod;
	module_table_idx = module_table_idx % MAX_MODULES;

	mod->init();
	uc_reg_write(uc, UC_X86_REG_EAX, &ret);
}

static
void emu_do_fini_module(uc_engine *uc, int fd, const char **params, int flags)
{
	if(module_table[fd] != NULL)
		emu_mod_release(module_table[fd]);
}

static void emu_do_munmap(uc_engine *uc, void *addr, size_t len)
{
	struct emu_mem_t *m = mem_list_head;
	int mem_id = (int) addr;

	dprintf("munmap: %d\n", (int) mem_id);

	while(m) {
		if(m->mem_id == mem_id) {
			mem_list_remove(m);
			break;
		}

		m = m->next;
	}
}

static void emu_do_mmap(uc_engine *uc,
	void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	int r_eax = MAP_FAILED;
	int id;
	void *p;

	uc_reg_write(uc, UC_X86_REG_EAX, &r_eax);
	if(!(flags & MAP_ANONYMOUS))
		return;

	if(len > MMAP_MAX_ALLOC)
		return;

	dprintf("mmap: Allocating %d bytes\n", len);
	
	p = (void*) malloc(len);
	if(p == NULL)
		return;

	mprotect(p, len, PROT_READ | PROT_WRITE | PROT_EXEC);
	id = mem_list_add(p, len);

	uc_reg_write(uc, UC_X86_REG_EAX, &id);
}

static
void emu_do_read(uc_engine *uc, int fd, void *buf, size_t size)
{
	struct emu_mem_t *m = mem_list_head;
	int ret = -1;

	uc_reg_write(uc, UC_X86_REG_EAX, &ret);

	while(m != NULL) {
		if(m->mem_id == fd)
			break;
		m = m->next;
	}

	if(!m)
		return;

	ret = size;
	uc_mem_write(uc, (uint64_t) buf, m->addr, size);
	uc_reg_write(uc, UC_X86_REG_EAX, &ret);
}

static
void emu_do_write(uc_engine *uc, int fd, void *buf, size_t size)
{
	struct emu_mem_t *m = mem_list_head;
	int ret = -1;
	size_t s;

	uc_reg_write(uc, UC_X86_REG_EAX, &ret);

	while(m != NULL) {
		if(m->mem_id == fd)
			break;
		m = m->next;
	}

	if(!m)
		return;
	
	s = size;
	if(s > m->len)
		s = m->len;
	
	if(!uc_mem_read(uc, (uint64_t) buf, m->addr, s)) {
		ret = s;
		uc_reg_write(uc, UC_X86_REG_EAX, &ret);
	}
}

static
void hook_intr(uc_engine *uc, uint32_t intno, void *user_data)
{
	int r_eax, r_ebx, r_ecx, r_edx, r_esi, r_edi, r_ebp;

	dprintf("Handling interrupt: %d\n", (int) intno);
	intrs_table[intno] += 1;

	if(intno == X86_SYSTEM_CALL) {
		r_eax = r_ebx = r_ecx = r_edx = r_esi = r_edi = r_ebp = 0;

		uc_reg_read(uc, UC_X86_REG_EAX, &r_eax);	
		uc_reg_read(uc, UC_X86_REG_EBX, &r_ebx);	
		uc_reg_read(uc, UC_X86_REG_ECX, &r_ecx);	
		uc_reg_read(uc, UC_X86_REG_EDX, &r_edx);
		uc_reg_read(uc, UC_X86_REG_ESI, &r_esi);
		uc_reg_read(uc, UC_X86_REG_EDI, &r_edi);
		uc_reg_read(uc, UC_X86_REG_EBP, &r_ebp);

		switch(r_eax) {
			case X86_SYS_EXIT:
				emu_do_exit(r_ebx);
				break;
			case X86_SYS_READ:
				emu_do_read(uc, r_ebx, r_ecx, r_edx);
				break;
			case X86_SYS_WRITE:
				emu_do_write(uc, r_ebx, r_ecx, r_edx);
				break;
			case X86_SYS_MMAP:
				emu_do_mmap(uc, r_ebx, r_ecx, r_edx, r_esi, r_edi, r_ebp);
				break;
			case X86_SYS_MUNMAP:
				emu_do_munmap(uc, r_ebx, r_ecx);
				break;
			case X86_SYS_INITMOD:
				emu_do_init_module(uc, r_ebx, r_ecx, r_edx);
				break;
			case X86_SYS_FINIMOD:
				emu_do_fini_module(uc, r_ebx, r_ecx, r_edx);
				break;
		}
	}
}

static
bool hook_memory_unmapped(uc_engine *uc, uc_mem_type type,
	uint64_t address, int size, int64_t value, void *user_data)
{
	bool ret = true;

	switch(type) {
		case UC_MEM_WRITE_UNMAPPED:
			dprintf("Unmapped memory write at 0x%"PRIx64" with size:%d value: %d\n",
				address, size, value);
			break;
		case UC_MEM_READ_UNMAPPED:
			dprintf("Unmapped memory read at 0x%"PRIx64" with size:%d value: %d\n",
				address, size, value);
			break;
		default:
			ret = false;
			break;
	}

	return ret;;
}

static
int emulate_verify_code(void *code, size_t len)
{
	uc_engine *uc;
	uc_err err;
	uc_hook trace_intr;
	uc_hook trace_ma;

	dprintf("Emulating %d bytes of code\n", (int) len);

	int r_esp = ADDRESS + STACK_OFFSET;
	err = uc_open(UC_ARCH_X86, UC_MODE_32, &uc);
	if(err) {
		dprintf("Failed to initialize unicorn engine. Err code: %u\n", err);
		return -1;
	}

	uc_mem_map(uc, ADDRESS, EMU_VM_SIZE, UC_PROT_ALL);
	if(uc_mem_write(uc, ADDRESS, code, len)) {
		dprintf("Failed to write code in emulation memory!\n");
		return -1;
	}

	uc_reg_write(uc, UC_X86_REG_ESP, &r_esp);
	uc_hook_add(uc, &trace_intr, UC_HOOK_INTR, hook_intr, NULL, 1, 0);
	uc_hook_add(uc, &trace_ma, UC_HOOK_MEM_READ_UNMAPPED | UC_HOOK_MEM_WRITE_UNMAPPED, 
		hook_memory_unmapped, NULL, 1, 0);

	err = uc_emu_start(uc, ADDRESS, ADDRESS + len, 0, 0);
	if(err) {
		dprintf("Emulation failed with error code: %u\n", err);
		return -1;
	}

	uc_close(uc);
	return 0;
}

int real_main(int argc, char **argv)
{
	int n = 0;
	int v = 0;
	void *p;

	dprintf("argv is at 0x%"PRIx64"\n", (void*) argv);

	n = read(FD_STDIN, &v, 4);
	if(n != 4)
		return -1;

	dprintf("Code len: %d\n", v);

	if(((unsigned int) v) > MAX_BUFFER)
		return -1;

	p = mmap(NULL, v, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, 0, 0);
	if(p == NULL)
		return -1;

	memset(p, 0, v);
	n = read(FD_STDIN, p, v);
	if(n < 0)
		return -1;

	if(emulate_verify_code(p, n)) {
		printf("ERR in emulation\n");
	}
	else {
		printf("DONE\n");
	}

	return 0;
}

int test_main(int a, char **b)
{
	uc_engine *uc;
	int ret;
	int i;

	uc_open(UC_ARCH_X86, UC_MODE_32, &uc);
	uc_mem_map(uc, ADDRESS, EMU_VM_SIZE, UC_PROT_ALL);

	ret = 0;
	emu_do_mmap(uc, NULL, 100, 0, MAP_ANON, -1, 0);	
	emu_do_mmap(uc, NULL, 100, 0, MAP_ANON, -1, 0);	
	emu_do_mmap(uc, NULL, 100, 0, MAP_ANON, -1, 0);	
	emu_do_mmap(uc, NULL, 100, 0, MAP_ANON, -1, 0);	
	emu_do_mmap(uc, NULL, 100, 0, MAP_ANON, -1, 0);	
	uc_reg_read(uc, UC_X86_REG_EAX, &ret);

	printf("EAX=%d\n", ret);

	emu_do_munmap(uc, (void*) 1, 100);
	emu_do_munmap(uc, (void*) 3, 100);
	emu_do_munmap(uc, (void*) 2, 100);
	emu_do_munmap(uc, (void*) 5, 100);
	emu_do_munmap(uc, (void*) 4, 100);

	ret = 0;
	emu_do_mmap(uc, NULL, 100, 0, MAP_ANON, -1, 0);	
	uc_reg_read(uc, UC_X86_REG_EAX, &ret);
	printf("EAX=%d\n", ret);

	// mem leak - need ret to have correct mem_id
	emu_do_read(uc, ret, ADDRESS, 0x1000);
	for(i = 0; i < (0x1000 / 4); i++) {
		uc_mem_read(uc, ADDRESS + (i * 4), &ret, 4);
		printf("%08x ", ret);
		
		if(!(i % 8))
			printf("\n");
	}

	// Must have 1 mem entry for this to succeed
	ret = 0xfefefefe;
	uc_mem_write(uc, ADDRESS, (void*) &ret, 4);
	emu_do_write(uc, mem_list_head->mem_id, ADDRESS, 4);
	uc_reg_read(uc, UC_X86_REG_EAX, &ret);
	printf("After emu_do_write EAX=%d\n", ret);
	printf("HEAD val: 0x%08x\n", *(unsigned int*) mem_list_head->addr);

	// UaF
	// Create holes
	for(i = 0;  i < 100; i++)
		emu_do_mmap(uc, NULL, sizeof(struct emu_module_t), 0, MAP_ANON, -1, 0);
	for(i = 0; i < 200; i++)
		emu_do_munmap(uc, (void*) i, 100);

	// Alloc and free target
	emu_do_init_module(uc, NULL, 0, NULL);
	uc_reg_read(uc, UC_X86_REG_EAX, &ret);
	printf("mod_fd EAX=%d\n", ret);

	emu_do_fini_module(uc, ret, NULL, 0);

	// Re-use free'd memory
	char buf[100];
	memset(buf, 0x41, sizeof(buf));

	for(i = 0; i < 200; i++) {
		uc_mem_write(uc, ADDRESS, (void*) &buf, sizeof(buf));
		emu_do_mmap(uc, NULL, sizeof(struct emu_module_t), 0, MAP_ANON, -1, 0);
		emu_do_write(uc, i, ADDRESS, sizeof(buf));
	}

	// Trigger UaF
	for(i = 0; i < 100; i++)
		emu_do_init_module(uc, NULL, 0, NULL);


	// Clear up mem list
	emu_do_munmap(uc, (void*) 2, 100);
	emu_do_munmap(uc, (void*) 5, 100);
	emu_do_munmap(uc, (void*) 4, 100);
	emu_do_exit(0);
}

//#define TEST_RUN

int main(int argc, char **argv)
{
#ifdef TEST_RUN
	return test_main(argc, argv);
#else
	return real_main(argc, argv);
#endif
}
