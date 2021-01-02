#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
// #include <elf.h>

#define max_str_len 32

enum {
	NOTYPE = 256, EQ, NEQ, AND, OR, MINUS, POINTER, NUMBER, HNUMBER, REGISTER, MARK

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{"\\b[0-9]+\\b",NUMBER,0},				// number
	{"\\b0[xX][0-9a-fA-F]+\\b",HNUMBER,0},		// 16 number
	{"\\$[a-zA-Z]+",REGISTER,0},				// register
	{"\\b[a-zA-Z_0-9]+", MARK, 0},		// mark
	{"!=",NEQ,3},						// not equal	
	{"!",'!',6},						// not
	{"\\*",'*',5},						// mul
	{"/",'/',5},						// div
	{"	+",NOTYPE,0},					// tabs
	{" +",NOTYPE,0},					// spaces
	{"\\+",'+',4},						// plus
	{"-",'-',4},						// sub
	{"==", EQ,3},						// equal
	{"&&",AND,2},						// and
	{"\\|\\|",OR,1},						// or
	{"\\(",'(',7},                        // left bracket   
	{"\\)",')',7},                        // right bracket 
	{"[0-9]+",NUMBER,0},				// number
	{"0[xX][0-9a-fA-F]+",HNUMBER,0},	// 16 number
	{"\\$[a-z,A-Z]+",REGISTER,0},		// register
	{"	+",NOTYPE,0},					// tabs
	{" +",NOTYPE,0},					// spaces
	{"\\|\\|",OR,1},					// or
	{"&&",AND,2},						// and
	{"!=",NEQ,3},						// not equal
	{"==", EQ,3},						// equal
	{"\\+",'+',4},						// plus
	{"-",'-',4},						// sub	
	{"\\*",'*',5},						// mul
	{"/",'/',5},						// div
	{"!",'!',6},						// not
	{"\\(",'(',7},                      // left bracket   
	{"\\)",')',7},                      // right bracket 
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
	int priority;
} Token;

Token tokens[32];
int nr_token;

uint32_t get_addr_from_mark(char *mark);

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
				char *tmp = e + position + 1;
				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE: break;
					case REGISTER:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority; 
						strncpy (tokens[nr_token].str,tmp,substr_len-1);
						tokens[nr_token].str[substr_len-1]='\0';
						nr_token ++;
						break; 
					default:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
						strncpy(tokens[nr_token].str,substr_start,substr_len);
						tokens[nr_token].str[substr_len]='\0';
						nr_token ++;
				}
				position += substr_len;

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

bool check_parentheses (int l,int r)
{
	int i;
	if (tokens[l].type == '(' && tokens[r].type ==')')
	{
		int lc = 0, rc = 0;
		for (i = l + 1; i < r; i ++)
		{
			if (tokens[i].type == '(')lc ++;
			if (tokens[i].type == ')')rc ++;
			if (rc > lc)return false;	
		}
		if (lc == rc)return true;
	}
	return false;
}

int dominant_operator (int l,int r)
{
	int i,j;
	int min_priority = 10;
	int oper = l;
	for (i = l; i <= r;i ++)
	{
		if (tokens[i].type == NUMBER || tokens[i].type == HNUMBER || tokens[i].type == REGISTER || tokens[i].type == MARK)
			continue;
		int cnt = 0;
		bool key = true;
		for (j = i - 1; j >= l ;j --)
		{ 
			if (tokens[j].type == '(' && !cnt) 
			{ 
				key = false;
				break; 
			}
			if (tokens[j].type == '(')cnt --;
			if (tokens[j].type == ')')cnt ++; 
		}
		if (!key)continue;
		if (tokens[i].priority <= min_priority) 
		{
			min_priority = tokens[i].priority;
			oper = i;
		}
 	}
	return oper;
}

uint32_t eval(int l,int r) {
	if (l > r)
	{
		Assert(0,"Bad expression!\n");
		return 0;
	}
	if (l == r) {
		uint32_t num = 0;
		if (tokens[l].type == NUMBER)
			sscanf(tokens[l].str,"%d",&num);
		if (tokens[l].type == HNUMBER)
			sscanf(tokens[l].str,"%x",&num);
		if (tokens[l].type == REGISTER)
		{
			if (strlen (tokens[l].str) == 3) 
			{
				int i;
				for (i = R_EAX; i <= R_EDI; i ++) {
					if (strcmp (tokens[l].str,regsl[i]) == 0) break;
				}
				if (i > R_EDI) 
				{
					if (strcmp (tokens[l].str,"eip") == 0) num = cpu.eip;
					else Assert(1,"no this register!\n");
				}
				else num = reg_l(i);
 			}

 			else if (strlen (tokens[l].str) == 2) 
			{
 				if (tokens[l].str[1] == 'x' || tokens[l].str[1] == 'p' || tokens[l].str[1] == 'i') 
				{
					int i;
					for (i = R_AX; i <= R_DI; i ++) 
					{
						if (strcmp (tokens[l].str,regsw[i]) == 0) break;
					}
					num = reg_w(i);
				}
 				else if (tokens[l].str[1] == 'l' || tokens[l].str[1] == 'h') 
				{
					int i;
					for (i = R_AL; i <= R_BH; i ++)
					{
						if (strcmp (tokens[l].str,regsb[i]) == 0)break;
					}
					num = reg_b(i);
				}
				else assert (1);
			}
		}

		if (tokens[l].type == MARK)
		{
			num = get_addr_from_mark(tokens[l].str);
			printf("%s", tokens[1].str);
		}
		
	
		return num;
	}

	else if (check_parentheses (l,r) == true) return eval (l + 1,r - 1);
 	else {
		int op = dominant_operator (l,r);
 		if (l == op || tokens[op].type == POINTER || tokens[op].type == MINUS || tokens[op].type == '!')
		{
			uint32_t val = eval (l + 1,r);
			switch (tokens[l].type)
 			{
				case POINTER:
					return swaddr_read(val,4, R_DS);
				case MINUS:
					return -val;
				case '!':
					return !val;
				default :
					Assert (1,"default\n");
			} 
		}

		uint32_t val1 = eval (l,op - 1);
		uint32_t val2 = eval (op + 1,r);
//		printf ("1 = %d,2 = %d\n",val1,val2);
		switch (tokens[op].type)
		{
			case '+':return val1 + val2;
			case '-':return val1 - val2;
			case '*':return val1 * val2;
			case '/':return val1 / val2;
			case EQ:return val1 == val2;
			case NEQ:return val1 != val2;
			case AND:return val1 && val2;
			case OR:return val1 || val2;
			default:
			break;
  		}
  	}
	assert (1);
	return -123456;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	int i;
	for (i = 0;i < nr_token; i ++) {
 		if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != NUMBER && tokens[i - 1].type != HNUMBER && tokens[i - 1].type != REGISTER && tokens[i - 1].type !=')' && tokens[i - 1].type != MARK))) {
			tokens[i].type = POINTER;
			tokens[i].priority = 6;
		}
		if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != NUMBER && tokens[i - 1].type != HNUMBER && tokens[i - 1].type != REGISTER && tokens[i - 1].type !=')' && tokens[i - 1].type != MARK))) {
			tokens[i].type = MINUS;
			tokens[i].priority = 6;
 		}
  	}
	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval(0,nr_token-1);
}
