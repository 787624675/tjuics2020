#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
uint32_t dram_read(hwaddr_t, size_t);
enum {
	NOTYPE = 256,          // spaces
	EQ,                    // equal
	DECINT,                // decimal integer
 	HEXINT,                // heximal integer
	REGISTER,		// register 
	LETTER,			// letter of lower case
	NOTEQ,			// not equal
	AND,
	OR,
	DEREF,                  // dereference
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"-", '-'},                                     // substract
	{"\\*", '*'},                                   // multiply
	{"/", '/'},                                     // division
	{"\\(", '('},					// left brace
	{"\\)", ')'},					// right brace
	{"0x", HEXINT},                                 // heximal integer
	{"[0-9]+", DECINT},				// decimal integer
	{"\\$", REGISTER},				// register
	{"[a-z]+", LETTER},				// letter
	{"!=", NOTEQ},					// not equal
	{"&&", AND},					// and
	{"\\|\\|", OR},					// or
	{"!", '!'},					// !
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					default: 
						Log("Unknown token type");
						break;
					case NOTYPE: 
						break;
					case '+':
						tokens[nr_token].type = '+';
						tokens[nr_token].str[0] = '+';
						nr_token ++;
						break;
					case EQ:
						tokens[nr_token].type = EQ;
						nr_token ++;
						break;
					case NOTEQ:
						tokens[nr_token].type = NOTEQ;
						nr_token ++;
						break;
					case '!':
						tokens[nr_token].type = '!';
						nr_token ++;
						break;
					case AND:
						tokens[nr_token].type = AND;
						nr_token ++;
						break;
					case OR:
						tokens[nr_token].type = OR;
						nr_token ++;
						break;
					case '-':
						tokens[nr_token].type = '-';
						tokens[nr_token].str[0]  = '-';
						nr_token ++;
						break;
					case '*':
						if(nr_token != 0 && tokens[nr_token-1].type != DECINT && tokens[nr_token-1].type != ')' && tokens[nr_token-1].type != HEXINT ){
						tokens[nr_token].type = DEREF;
			// Don't add 1 to nr_token, waiting for the input of the var's name
}else{
						tokens[nr_token].type = '*';
						tokens[nr_token].str[0]  = '*';
						nr_token ++;
}
						break;
					case '/':
						tokens[nr_token].type = '/';
						tokens[nr_token].str[0] = '/';
						nr_token ++;
						break;
					case '(':
						tokens[nr_token].type = '(';
						tokens[nr_token].str[0] = '(';
						nr_token ++;
						break;
					case ')':
						tokens[nr_token].type = ')';
						tokens[nr_token].str[0] = ')';
						nr_token ++;
						break;
					case DECINT:
						memset(tokens[nr_token].str, 0, 32);
						if(tokens[nr_token].type == HEXINT){
							// Nothing changes
					// Is it initially null? If not, there will be bugs 
						}else{
							tokens[nr_token].type = DECINT;
						}
						strncpy(tokens[nr_token].str, substr_start, substr_len );
						/* handle the case when the buffer will be fluided*/
						nr_token ++;
						if(substr_len > 32) {
						/* exit directly*/
							panic("the decimal int is too long");
						}
						break;
					case HEXINT:
						tokens[nr_token].type = HEXINT;
						// Maybe it is needless to put "0x" into str
					// There must be a number then, so don't add 1 to nr_token
						break;
					case REGISTER:
						tokens[nr_token].type = REGISTER;
						break;
					case LETTER:
						memset(tokens[nr_token].str, 0, 32);
						strncpy(tokens[nr_token].str, substr_start, substr_len );
						nr_token ++;	
						break;

				}
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses(int  p, int q){
	
	if(tokens[p].str[0]!='('||tokens[q].str[0]!=')'){
		return false;
	}
	int temp_q = q;
	while(p <= temp_q){
		if(tokens[p].str[0] == '('){
				while(q >= 0){
					if(tokens[q].str[0]==')'){
						q--;
						break;
					}else{
						q--;
						if(q <= p){
							return false;
						}
					}
				}
				p++;
			}else{
				p++;
				if(p >= temp_q){
					return true;
					break;
				}
			}
	}
	return true;
}

uint32_t eval(p ,q){
	if(p > q){
		/* Bad eapression*/
		return 0;
	}
	else if(p == q) {
		char *str;
		uint32_t number = 0;
		if(tokens[p].type == DECINT){
			number = strtol(tokens[p].str, &str, 10);
		}else if(tokens[p].type == HEXINT){
			number = strtol(tokens[p].str, &str, 16);
		}else if(tokens[p].type == REGISTER){
			if(strcmp(tokens[p].str, "eax") == 0){
				number = cpu.eax;
			}
			else if(strcmp(tokens[p].str, "ecx") == 0){
				number = cpu.ecx ;
			}
			else if(strcmp(tokens[p].str, "edx") == 0){
				number = cpu.edx ;
			}
			else if(strcmp(tokens[p].str, "ebx") == 0){
				number = cpu.ebx ;
			}
			else if(strcmp(tokens[p].str, "esp") == 0){
				number = cpu.esp ;
			}
			else if(strcmp(tokens[p].str, "ebp") == 0){
				number = cpu.ebp ;
			}
			else if(strcmp(tokens[p].str, "esi") == 0){
				number = cpu.esi ;
			}

			else if(strcmp(tokens[p].str, "edi") == 0){
				number = cpu.edi ;
			}
			else{   // Suddenly find that it can be wrote like this...	
				int index;
				for(index = 0; index < 8; index++){
					if(strcmp(tokens[p].str, regsw[index]) == 0){
						number = reg_w(index);
					}
				}
				for(index = 0; index < 8; index++){
					if(strcmp(tokens[p].str, regsb[index]) == 0){
						number = reg_b(index);
					}
				}
			}
		}else if(tokens[p].type == DEREF){
			number = dram_read(eval(p,p), 1);
}
		return number;	
	}
	else if(check_parentheses(p, q)==true) {
		return eval(p+1, q-1);
	}
	else{
		int p_find = p;
		int in_brace = 0;
		p_find = p;
		while(p_find <= q){
			if(tokens[p_find].str[0] == '('){ in_brace ++ ;}
			if(tokens[p_find].str[0] == ')'){ in_brace -- ;}		
			if(tokens[p_find].type == OR && in_brace == 0){
				return eval(p, p_find-1) || eval(p_find+1, q);
			}
			p_find++;
		}
		p_find = p;
		while(p_find <= q){
			if(tokens[p_find].str[0] == '('){ in_brace ++ ;}
			if(tokens[p_find].str[0] == ')'){ in_brace -- ;}		
			if(tokens[p_find].type == AND && in_brace == 0){
				return eval(p, p_find-1) && eval(p_find+1, q);
			}
			p_find++;
		}
		p_find = p;
		while(p_find <= q){
			if(tokens[p_find].str[0] == '('){ in_brace ++ ;}
			if(tokens[p_find].str[0] == ')'){ in_brace -- ;}		
			if(tokens[p_find].type == EQ && in_brace == 0){
				return eval(p, p_find-1) == eval(p_find+1, q);
			}
			else if(tokens[p_find].type == NOTEQ && in_brace == 0){
			
				return eval(p, p_find-1) != eval(p_find+1, q);
			}
			p_find++;
		}
		p_find = p;
		while(p_find <= q){
			if(tokens[p_find].str[0] == '('){ in_brace ++ ;}
			if(tokens[p_find].str[0] == ')'){ in_brace -- ;}		
			if(tokens[p_find].str[0] == '+' && in_brace == 0){
				return eval(p, p_find-1) + eval(p_find+1, q);
			}
			else if(tokens[p_find].str[0] == '-' && in_brace == 0){
				// The case that '-' means minus
				if(p_find == p){
					return 0 - eval(p+1, q);
				}
				else if(tokens[p_find-1].type=='*'||tokens[p_find-1].type=='/')
				{
				// let the * and / handle it, since their piority is lower
				}else{
					return eval(p, p_find-1) - eval(p_find+1, q);
				}
			}
			p_find++;
		}	
		p_find = p;
		in_brace = 0;
		while(p_find <= q){
			if(tokens[p_find].str[0] == '('){ in_brace ++ ;}
			if(tokens[p_find].str[0] == ')'){ in_brace -- ;}	
			if(tokens[p_find].str[0] == '*' && in_brace == 0){
				return eval(p, p_find-1) * eval(p_find+1, q);
			}
			else if(tokens[p_find].str[0] == '/' && in_brace == 0){
				return eval(p, p_find-1) / eval(p_find+1, q);
			}
			p_find++;
		}
		if(tokens[p].type == '!'){
			return !eval(p+1, q);
		}
	}
	return 0;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval(0, nr_token-1);
}
