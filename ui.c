#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

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

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	/* TODO: Add more commands */
	{ "si", "Execute by N steps, if N is 1 by default", cmd_si },
	{ "info", "Print information of registers", cmd_info },
	{ "x", "Scan the memory", cmd_x },
	{ "p", "Evaluate the expression", cmd_p},
	{ "w", "Add Watch point", cmd_w},
	{ "d", "Delete Watch point", cmd_d},

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

static int cmd_si(char *args){
	/* Extract the first argument */
	char *arg = strtok(NULL, " ");
	/* if arg == NULL, execute one step*/
	if(arg == NULL) {
		/* Execute by one step */
		cpu_exec(1);
	} 
	else {
		char* ptr;
		uint32_t N = strtol(arg,&ptr,10);	
		cpu_exec(N);
	}
	return 0;
}

WP* get_wp();

static int cmd_info(char *args){
	char *arg = strtok(NULL, " ");
	if(arg == NULL){
		printf("Too few arguements for function info\n");
	}
	/* Allow user to type r or register */
	else if(strcmp(arg, "register")==0||strcmp(arg, "r")==0){
		printf("eax: %d \n",cpu.eax);
		printf("ecx: %d \n",cpu.ecx);
		printf("edx: %d \n",cpu.edx);
		printf("ebx: %d \n",cpu.ebx);
		printf("esp: %d \n",cpu.esp);
		printf("ebp: %d \n",cpu.ebp);
		printf("esi: %d \n",cpu.esi);
		printf("edi: %d \n",cpu.edi);
		printf("ax: %d \n",reg_w(0));
		printf("cx: %d \n",reg_w(1));
		printf("dx: %d \n",reg_w(2));
		printf("bx: %d \n",reg_w(3));
		printf("sp: %d \n",reg_w(4));
		printf("bp: %d \n",reg_w(5));
		printf("si: %d \n",reg_w(6));
		printf("di: %d \n",reg_w(7));
		printf("al: %d \n",reg_b(R_AL));
		printf("cl: %d \n",reg_b(R_CL));
		printf("dl: %d \n",reg_b(R_DL));
		printf("bl: %d \n",reg_b(R_BL));
		printf("ah: %d \n",reg_b(R_AH));
		printf("ch: %d \n",reg_b(R_CH));
		printf("dh: %d \n",reg_b(R_DH));
		printf("bh: %d \n",reg_b(R_BH));

	}else if(strcmp(arg, "watchpoints")==0||strcmp(arg, "w")==0){
		WP* p;
		p = get_wp();
		if(p == NULL){
			printf("There is no watchpoints\n");
		}else{
			int i = 0;
			do{
				i++;
				printf("NO.%d\texpression:%s\tvalue:%d\n", p->NO, p->expr,p->value);
				p = p->next;
			}while(p!=NULL);

		}
	}
	return 0;
}

uint32_t dram_read(hwaddr_t addr, size_t len);
 
static int cmd_x(char *args){
	char *arg = strtok(NULL, " ");
	char *arg1 = strtok(NULL, " ");
	char *ptr;
	size_t steps = strtol(arg,&ptr, 10);
	uint32_t start_addr = strtol(arg1,&ptr, 16);
	uint32_t read_result = dram_read(start_addr, steps); 
	printf("0x%x:\t",start_addr);
	int i;
	for (i = 0; i < steps; i++) {
		printf("%d \t", read_result>>i*4);
	}
	return 0;
}

static int cmd_p(char *args){
	bool call_back;
	if(args==NULL){
		printf("Please input the expression\n");
		return 0;
	}else{
		uint32_t result = expr(args, &call_back);
		if(call_back == true){
			printf("%d\n", result);
			return 0;
		}
		else{
			printf("Failed to evaluate this expression.");
			return 0;
		}
	}
}

WP* new_wp();

static int cmd_w(char *args){
	WP* wp = new_wp();
	bool callback;
	if(strlen(args) >= 32){
		panic("Too long the expression is");
	}
	strcpy(wp -> expr , args); 
	wp -> value = expr(args, &callback);
	if(callback == false){
		printf("Faild to evaluate the expression, the result is 0 now");
	}
	
	return 0;
} 
void free_wp(uint32_t num);
static int cmd_d(char *args){
	char* arg = strtok(NULL," ");
	char *str;
	uint32_t num = strtol(arg, &str, 10 );
	free_wp(num);
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
