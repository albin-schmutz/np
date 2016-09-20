#include <stdio.h>
#include <stdlib.h>
#include "opcodes.h"

#define WORD_SIZE 4
#define NBR_REGS 16
#define BIT_OC 6
#define BIT_REG 4
#define BIT_OC_MASK 0x3f
#define BIT_REG_MASK 0xf
#define JMP PC += c * WORD_SIZE

/* args */

static int args_count;
static char **args;

/* runtime registers */

#define GP r[NBR_REGS - 5]
#define SP r[NBR_REGS - 3]
#define RT r[NBR_REGS - 2]
#define PC r[NBR_REGS - 1]

static int r[NBR_REGS];

/* runtime flags */

static int z, n;

/* dynamic memory address*/

static int heap;


static FILE *open_file(char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f) {
		perror(filename);
		exit(1);
	}
	return f;
}

static char *alloc_memory(int size)
{
	char *m = malloc(size);
	if (!m) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	return m;
}

static void dump_regs(void)
{
	int i;
	fprintf(stderr, "<dump_regs>\n");
	for (i = 0; i < NBR_REGS; ++i) {
		fprintf(stderr, "%02i: %08x  ", i, r[i]);
		if (i % 4 == 3) fprintf(stderr, "\n");
	}
}

/*
 *
 * single memory model
 *
 *         ---------
 *         |       |
 *         |  dym  |      dynamic memory (heap)
 *         |       |
 *         ---------
 *         |       |  <-- program epilog (var-size, entry-point)
 *         |       |
 * PC -->  |  prg  |      program memory
 *         |       |
 * GP -->  ---------
 *         |       |      static memory
 * SP -->  |  stm  |      (global variables & stack)
 *         |       |
 * m -->   ---------
 *
 */

static void create_single_memory_model(FILE *file)
{
	int MB = 1024 * 1024;
	int total_memory_size = 2 * MB;
	char *m = alloc_memory(total_memory_size);
	int static_memory_size = 1 * MB;
	int program_memory_size = fread(
		m + static_memory_size,
		1,
		total_memory_size - static_memory_size,
		file);
	heap = (int)(m + static_memory_size + program_memory_size);
	char *program_epilog = m + static_memory_size +
		program_memory_size - 2 * WORD_SIZE;
	int variable_memory_size = *((int*)program_epilog);
	int entry_point_offset = *((int*)(program_epilog + WORD_SIZE));
	GP = (int)(m + static_memory_size);
	SP = GP + variable_memory_size;
	PC = GP + entry_point_offset;
	printf("memory-model: single\n");
	printf("total memory size (MB): %i\n", total_memory_size / MB);
	printf("start address memory: %p\n", m);
	printf("static memory size (MB): %i\n", static_memory_size / MB);
	printf("program memory size (B): %i\n", program_memory_size);
	printf("start address program epilog: %p\n", program_epilog);
	printf("variable memory size (B): %i\n", variable_memory_size);
	printf("entry point offset: %i\n", entry_point_offset);
	printf("GP: %08x\n", GP);
	printf("SP: %08x\n", SP);
	printf("PC: %08x\n", PC);
}

/* sys calls */

struct STACK_1 { int p0; };
struct STACK_2 { int p1; int p0; };
struct STACK_3 { int p2; int p1; int p0; };

static void sys_get_info(int *res, struct STACK_1 *gi)
{
	switch (gi->p0) {
	case 1:
		*res = heap;
		break;
	case 2:
		*res = args_count;
		break;
	case 3:
		*res = (int)stdin;
		break;
	case 4:
		*res = (int)stdout;
		break;
	case 5:
		*res = (int)stderr;
		break;
	default:
		fprintf(stderr, "get info %i ???\n", gi->p0);
		exit(1);
	}
}

static void sys_open_file(int *res, struct STACK_2 *of)
{
	switch (of->p1) {
	case 1:
		*res = (int)fopen((char*)(of->p0), "rb");
		break;
	case 2:
		*res = (int)fopen((char*)(of->p0), "wb");
		break;
	default:
		fprintf(stderr, "open file mode %i ???\n", of->p1);
		exit(1);
	}
}

/* p0: file, p1: addr, p2: len */
static void sys_read_file(int *res, struct STACK_3 *f)
{
	*res = fread((char*)(f->p1), 1, f->p2, (FILE*)(f->p0));
}

/* p0: file, p1: addr, p2: len */
static void sys_write_file(int *res, struct STACK_3 *f)
{
	*res = fwrite((char*)(f->p1), 1, f->p2, (FILE*)(f->p0));
}

static void sys(int nr, int *res, int par)
{
	switch (nr) {
	case -5:
		dump_regs();
		break;
	case -4:
		break;
	case -3:
		putchar(*((int*)(GP - 4)));
		break;
	case -2:
		break;
	case -1:
	case 0:
		exit(((struct STACK_1*)par)->p0);
		break;
	case 1:
		sys_get_info(res, (struct STACK_1*)par);
		break;
	case 2:
		*res = (int)args[((struct STACK_1*)par)->p0];
		break;
	case 11:
		sys_open_file(res, (struct STACK_2*)par);
		break;
	case 12:
		fclose((FILE*)(((struct STACK_1*)par)->p0));
		break;
	case 13:
		sys_read_file(res, (struct STACK_3*)par);
		break;
	case 14:
		sys_write_file(res, (struct STACK_3*)par);
		break;
	default:
		fprintf(stderr, "sys %i ???\n", nr);
		exit(1);
	}
}

/* bytecode interpreter */

static void run(void)
{
	/* push return address 0 */
	SP -= WORD_SIZE; *((int*)SP) = 0;
	while (PC) {
		int ir = *((int*)PC);
		/*fprintf(stderr, "PC:%08x(%04x) ir:%08x oc:%i\n",
			PC, (PC - GP), ir, ir & BIT_OC_MASK);*/
		PC += WORD_SIZE;
		int oc = ir & 0x3f;
		int a, b, c;
		if (oc < 48) {
			a = (ir >> BIT_OC) & BIT_REG_MASK;
			b = (ir >> (BIT_OC + BIT_REG)) & BIT_REG_MASK;
			if (oc < 32) {
				c = ir >> (BIT_OC + BIT_REG + BIT_REG);
				if (oc < 16) c = r[c];
			} else {
				c = *((int*)PC);
				PC += WORD_SIZE;
			}
		} else {
			c = ir >> BIT_OC;
		}
		switch (ir & BIT_OC_MASK) {
		case OC_MOV: case OC_MOVI: case OC_MOVI2:
			r[a] = c << b;
			break;
		case OC_MVN: case OC_MVNI: case OC_MVNI2:
			r[a] = -(c << b);
			break;
		case OC_CMP: case OC_CMPI: case OC_CMPI2:
			z = (r[a] == c);
			n = (r[a] < c);
			break;
		case OC_MUL: case OC_MULI: case OC_MULI2:
			r[a] = r[b] * c;
			break;
		case OC_DIV: case OC_DIVI: case OC_DIVI2:
			r[a] = r[b] / c;
			break;
		case OC_MOD: case OC_MODI: case OC_MODI2:
			r[a] = r[b] % c;
			break;
		case OC_ADD: case OC_ADDI: case OC_ADDI2:
			r[a] = r[b] + c;
			break;
		case OC_SUB: case OC_SUBI: case OC_SUBI2:
			r[a] = r[b] - c;
			break;
		case OC_SYS:
			sys(c, &r[a], r[b]);
			break;
		case OC_LDB:
			r[a] = *((char*)(r[b] + c));
			break;
		case OC_LDW:
			r[a] = *((int*)(r[b] + c));
			break;
		case OC_STB:
			*((char*)(r[b] + c)) = r[a];
			break;
		case OC_STW:
			*((int*)(r[b] + c)) = r[a];
			break;
		case OC_POP:
			r[a] = *((int*)r[b]);
			r[b] += c;
			break;
		case OC_PSH:
			r[b] += c;
			*((int*)r[b]) = r[a];
			break;
		case OC_JUMP:
			JMP;
			break;
		case OC_CALL:
			RT = PC; JMP;
			break;
		case OC_BEQ:
			if (z) JMP;
			break;
		case OC_BNE:
			if (!z) JMP;
			break;
		case OC_BLS:
			if (n) JMP;
			break;
		case OC_BGE:
			if (!n) JMP;
			break;
		case OC_BLE:
			if (z || n) JMP;
			break;
		case OC_BGT:
			if (!z && !n) JMP;
			break;
		default:
			fprintf(stderr, "oc %i ???\n", ir & BIT_OC_MASK);
			exit(1);
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc >= 2 ) {
		args_count = argc - 2;
		args = argc > 2 ? &argv[2] : NULL;
		FILE *file = open_file(argv[1]);
		create_single_memory_model(file);
		fclose(file);
		run();
	} else {
		fprintf(stderr, "\nusage: npx <bytecode-file>\n\n");
	}
	return 0;
}
