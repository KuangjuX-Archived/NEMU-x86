#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

typedef struct {
	swaddr_t prev_ebp;
	swaddr_t ret_addr;
	uint32_t args[4];
} PartOfStackFrame;

void get_func_from_addr(char *tmp, swaddr_t addr);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
	int num = 0;
	if (args==NULL) {
		// printf("Need more arguments.\n");
		num = 1;
	} else {
		sscanf(args, "%d", &num);
	}
	cpu_exec(num);
	// printf("Done.");
	return 0;
};

static int cmd_info(char *args) {
	char *arg = strtok(args, " ");
	// printf("%s\n", arg);
	if (strcmp(arg, "r") == 0)
	{
		int i;
		for ( i = R_EAX; i <= R_EDI; i++)
		{
			printf("%s\t0x%08x\n", regsl[i], reg_l(i));
		}
		
	}

	if (strcmp(arg, "w") == 0)
	{
		printf("Will print the watch point.\n");
		info_wp();
	}
	
	
	return 0;
};

static int cmd_x(char *args) {
	if (args == NULL)
	{
		printf("Need more parameters.\n");
		return 1;
	}
	
	char *arg = strtok(args, " ");
	if (arg == NULL)
	{
		printf("Need more parameters.\n");
		return 1;
	}

	int n = atoi(arg);
	char *EXPR = strtok(NULL, " ");
	if (EXPR == NULL)
	{
		printf("Need more parameters.\n");
	}


	char *str;
	swaddr_t address = strtol(EXPR, &str, 16);

	// Scan
	int i;
	int j;
	for (i = 0; i < n; i++)
	{
		uint32_t data = swaddr_read(address+i*4, 4, R_DS);
		printf("0x%08x: ", address + i * 4);

		for (j = 0; j < 4; j++)
		{
			printf("0x%02x ", data & 0xff);
			data = data >> 8; /*4 bytes each time*/
		}
		printf("\n");
	}
	
	return 0;
	
};

static int cmd_p(char *args) {
	uint32_t num;
	bool success;
	num = expr(args, &success);
	if (success)
	{
		printf("Expression %s:\t0x%x\t%d\n", args, num, num);
	}
	else assert(0);
	return 0;
};

static int cmd_w(char *args) {
	WP *f;
	bool suc;
	f = new_wp();
	printf("Watch-point %d: %s is set\n", f->NO, args);
	f->value = expr(args, &suc);
	strcpy(f->expr, args);
	if (suc == false) Assert(1, "Evaluation failed!");
	printf("Value: %d\n", f->value);
	return 0;
}

static int cmd_d(char *args) {
	char* arg = strtok(args, " ");
	int num;
	if (arg==NULL) {
		printf("Need more arguments, please inset an integer to appoint the watch-point.\n");
	} else {
		num = atoi(arg);
		delete_wp(num);
	}
	return 0;
}

static void read_ebp(swaddr_t addr, PartOfStackFrame *ebp) {
	ebp->prev_ebp = swaddr_read(addr, 4, R_SS);
	ebp->ret_addr = swaddr_read(addr+4, 4, R_SS);
	int i;
	for ( i = 0; i < 4; i++)
	{
		ebp->args[i] = swaddr_read(addr+8+4*i, 4, R_SS);
	}
	
}

static int cmd_bt(char *args) {
	int j = 0;
	PartOfStackFrame current_ebp;
	char tmp[32];
	
	swaddr_t addr = reg_l(R_EBP);
	current_ebp.ret_addr = cpu.eip;
	while (addr > 0)
	{
		printf("#%d 0x%08x in\t", j++, current_ebp.ret_addr);
		
		get_func_from_addr(tmp, current_ebp.ret_addr);

		printf("%s\t", tmp);
		read_ebp(addr, &current_ebp);
		if (strcmp(tmp, "main") == 0) printf("( )\n");
		else {
			printf("( %d, %d, %d, %d )\n", current_ebp.args[0], current_ebp.args[1], current_ebp.args[2], current_ebp.args[3]);
		}
		addr = current_ebp.prev_ebp;
	}
	return 0;
	
}

static int cmd_page(char *args) {
	if (args == NULL) return 0;
	lnaddr_t lnaddr;
	sscanf(args, "%x", &lnaddr);
	hwaddr_t hwaddr = page_translate(lnaddr, 1);
	if (!cpu.cr0.protect_enable || !cpu.cr0.paging)
	{
		printf("\033[1;33mPage address convertion is invalid.\n\033[0m");
	}
	printf("0x%x -> 0x%x\n", lnaddr, hwaddr);
	return 0;
	
}

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Step into implementation of N instructions after the execution with a default value of 1 when N is not given.", cmd_si},
	{ "info", "r: print the state of registers.\nw: print watch point position.", cmd_info},
	{ "x", "Caculate the result of the expression and print continuous N byte in hex started with the value.", cmd_x},

	{ "p", "Expression evaluation.", cmd_p},
	{ "w", "Set up a watch-point to detect if the value is changed.", cmd_w},
	{ "d", "Delete a watch-point", cmd_d},
	{ "bt", "Print stack frame chain.", cmd_bt},
	{ "page", "Convert virtual address to physical address", cmd_page},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
