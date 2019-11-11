#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

//sent back register for store xyz

#define MAX_LENGTH 100

int x_count = 0;
int y_count = 0;
int z_count = 0;
int x_PostInc = 0;
int y_PostInc = 0;
int z_PostInc = 0;
int x_PostDec = 0;
int y_PostDec = 0;
int z_PostDec = 0;
int xRegChange = 0;
int yRegChange = 0;
int zRegChange = 0;

typedef struct regist{
	int status;
	int value;
	char regis[5];
}regist;


// Token / AST kinds
enum {
	// Non-operators
	LPar, RPar, // parentheses
	Value, Variable,
	// Operators
	/// Precedence 1
	PostInc, PostDec,
	/// Precedence 2
	PreInc, PreDec, Plus, Minus,
	/// Precedence 3
	Mul, Div, Rem,
	/// Precedence 4
	Add, Sub,
	/// Precedence 14
	Assign
};
const char TYPE[20][20] = {
	"Par", "Par",
	"Value", "Variable",
	"PostInc", "PostDec",
	"PreInc", "PreDec", "Plus", "Minus",
	"Mul", "Div", "Rem",
	"Add", "Sub",
	"Assign"
};
typedef struct _TOKEN {
	char tmp_reg[10];
	int kind;
	int param; // Value, Variable, or Parentheses label
	struct _TOKEN *prev, *next;
} Token;
typedef struct _AST {
	char tmp_reg[10];
	int type;
	int val; // Value or Variable
	struct _AST *lhs, *rhs, *mid;
} AST;

// Utility Interface

// Function called when an unexpected expression occurs.
void err();
// Used to create a new Token.
Token *new_token(int kind, int param);
// Used to create a new AST node.
AST *new_AST(Token *mid);
// Convert Token linked list into array form.
int list_to_arr(Token **head);

// Pass "kind" as parameter. Return true if it is an operator kind.
int isOp(int x);
// Pass "kind" as parameter. Return true if it is an unary kind.
// unary contains increment, decrement, plus, and minus.
int isUnary(int x);
// Pass "kind" as parameter. Return true if it is a parentheses kind.
int isPar(int x);
// Pass "kind" as parameter. Return true if it is a plus or minus.
int isPlusMinus(int x);
// Pass "kind" as parameter. Return true if it is an operand(value or variable).
int isOperand(int x);
// Return the precedence of a kind. If doesn't have precedence, return -1.
int getOpLevel(int kind);
// Given the position of left parthesis, find the right part in range [tar,r]. If not found, return -1.
int findParPair(Token *arr, int tar, int r);
// Parse the expression the the next section. A section means a Value, Variable, or a parenthesis pair.
int nextSection(Token *arr, int l, int r);
// Used to find a appropriate operator that split the expression half.
int find_Tmid(Token *arr, int l, int r);
// Determine the memory location of variable
int var_memory(AST *ast);

// Debug Interface

// Print the AST. You may set the indent as 0.
void AST_print(AST *head, int indent);

void resetStatus(regist*);

// Main Function

char input[MAX_LENGTH];

// Convert the inputted string into multiple tokens.
Token *lexer(char *in);
// Use tokens to build the binary expression tree.
AST *parser(Token *arr, int l, int r);
// Checkif the expression(AST) is legal or not.
void semantic_check(AST *now);
// Generate the ASM.
void codegen(AST *ast);

AST *ast_root;
regist *r ;
int main() {
	r = (regist*)malloc(sizeof(regist)*256);
	for(int i=0;i<256;i++){
		
		char tmp[10] = "r";
		char tmp_1[10] ;
		sprintf(tmp_1,"%d",i);
		//itoa(i,tmp_1,10);
		strcat(tmp,tmp_1);
		strcpy(r[i].regis,tmp);
		r[i].status = 0;
		//puts(r[i].regis);
	}
	while(fgets(input, MAX_LENGTH, stdin) != NULL) {
		//x_count = y_count = z_count = 0;
		x_PostInc = x_PostInc = y_PostInc = y_PostDec = z_PostInc = z_PostDec =0;
		// build token list by lexer
		Token *content = lexer(input);
		// convert token list into array
		int length = list_to_arr(&content);
		// build abstract syntax tree by parser
		ast_root = parser(content, 0, length-1);
		AST *tmp = ast_root;
		// check if the syntax is correct
		semantic_check(tmp);
		// generate the assembly
		codegen(tmp);

		if(r[0].status ==2 || r[1].status == 2 ||r[2].status == 2){
			if(r[0].status==2 ){
				if(x_PostInc) puts("add r0 r0 1");
				else if(x_PostDec) puts("sub r0 r0 1");
				//printf("add r0 r0 1\n");
				printf("store %s %s\n","[0]","r0");
			}
			else if(r[1].status==2){
				if(y_PostInc) puts("add r1 r1 1");
				else if(y_PostDec) puts("sub r1 r1 1");
				printf("store %s %s\n","[4]","r1");
			}
			else if(r[2].status ==2){
				if(z_PostInc) puts("add r2 r2 1");
				else if(z_PostDec) puts("sub r2 r2 1");
				printf("store %s %s\n","[8]","r2");
			}
		}
		resetStatus(r);
		//AST_print(ast_root,0);
		
	}
	free(r);
	return 0;
}

Token *lexer(char *in) {
	Token *head = NULL, *tmp = NULL;
	Token **now = &head, *prev = NULL;
	int par_cnt = 0;
	for(int i = 0; in[i]; i++) {
		if(in[i] == ' ' || in[i] == '\n')
			continue;

		else if('x' <= in[i] && in[i] <= 'z')
			(*now) = new_token(Variable, in[i]);

		else if(isdigit(in[i])) {
			int val = 0, oi = i;
			for(; isdigit(in[i]); i++)
				val = val * 10 + (in[i] - '0');
			i--;
			// Detect illegal number inputs such as "01"
			if(oi != i && in[oi] == '0')
				err();
			(*now) = new_token(Value, val);
		}

		else {
			switch(in[i]) {
				case '+':
					if(in[i+1] == '+') { // '++'
						tmp = prev;
						while(tmp != NULL && tmp->kind == RPar) tmp = tmp->prev;
						if(tmp != NULL && tmp->kind == Variable)
							(*now) = new_token(PostInc, 0);
						else (*now) = new_token(PreInc, 0);
						i++;
					}
					else { // '+'
						if(prev == NULL || isOp(prev->kind)||prev->kind == LPar || isPlusMinus(prev->kind))
							(*now) = new_token(Plus, 0);
						else (*now) = new_token(Add, 0);
					}
					break;
				case '-':
					if(in[i+1] == '-') { // '--'
						tmp = prev;
						while(tmp != NULL && tmp->kind == RPar) tmp = tmp->prev;
						if(tmp != NULL && tmp->kind == Variable)
							(*now) = new_token(PostDec, 0);
						else (*now) = new_token(PreDec, 0);
						i++;
					}
					else { // '-'
						if(prev == NULL || isOp(prev->kind)||prev->kind == LPar || isPlusMinus(prev->kind))
							(*now) = new_token(Minus, 0);
						else (*now) = new_token(Sub, 0);
					}
					break;
				case '*':
					(*now) = new_token(Mul, 0);
					break;
				case '/':
					(*now) = new_token(Div, 0);
					break;
				case '%':
					(*now) = new_token(Rem, 0);
					break;
				case '(':
					(*now) = new_token(LPar, par_cnt++);
					break;
				case ')':
					(*now) = new_token(RPar, --par_cnt);
					break;
				case '=':
					(*now) = new_token(Assign, 0);
					break;
				default:
					err();
			}
		}
		(*now)->prev = prev;
		if(prev != NULL) prev->next = (*now);
		prev = (*now);
		now = &((*now)->next);
	}
	return head;
}

void resetStatus(regist *r){
	for(int i=0;i<256;i++){
		r[i].status = 0;
	}
}

AST *parser(Token *arr, int l, int r) {
	if(l > r) return NULL;
	// covered in parentheses
	if(r == findParPair(arr, l, r)) {
		AST *res = new_AST(arr+l);
		res->mid = parser(arr, l+1, r-1);
		return res;
	}

	int mid = find_Tmid(arr, l, r);
	AST *newN = new_AST(arr + mid);
	
	if(l == r || newN->type <= Variable){
		if(l == r && newN->type <= Variable){
			return newN;
		}
		else{
			err();
		}
	}
		

	if(getOpLevel(arr[mid].kind) == 1) // a++, a--
		newN->mid = parser(arr, l, mid-1);
	else if(getOpLevel(arr[mid].kind)== 2){ //++a --a
		newN->mid = parser(arr,mid+1,r);
	}
	else if(getOpLevel(arr[mid].kind == 3)){ //
		newN->lhs = parser(arr,l,mid-1);
		newN->rhs = parser(arr,mid+1,r);

	}
	else if(getOpLevel(arr[mid].kind == 4)){
		newN->lhs = parser(arr,l,mid-1);
		newN->rhs = parser(arr,mid+1,r);
	}

	// TODO: Implement the remaining parser part.
	// hint: else if(other op type?) then do something ..etc
	return newN;
}

void semantic_check(AST *now) {
	if(isUnary(now->type) || isPar(now->type)) {
		
		if(now->lhs != NULL || now->rhs != NULL)
			err();
		if(now->mid == NULL)
			err();
		if(isUnary(now->type)) {
			AST *tmp = now->mid;
			if(isPar(tmp->type)) {
				while(isPar(tmp->type))
					tmp = tmp->mid;
			}
			if(isPlusMinus(now->type)) {
				if(isUnary(tmp->type));
				else if(isOperand(tmp->type));
				else err();
			}
			else if(tmp->type != Variable)
				err();
		}

		semantic_check(now->mid);
	}
	else if(isOp(now->type)||isOperand(now->type)){
		if(isOp(now->type)){
			if(now->lhs == NULL || now->rhs == NULL){
				err();
			}
			semantic_check(now->lhs);
			semantic_check(now->rhs);
			
		}
		else if(isOperand(now->type)){
			if(now->lhs != NULL || now->rhs != NULL){
			err();
			}
		}
		
	} 

	// TODO: Implement the remaining semantic check part.
	// hint: else if(other op type?) then do something ...etc*/
}

void find_reg(AST**a){
	for(int i=3;i<236;i++){ //r0 r1 r2 set for x y z. Other operation start from r3.
		if(r[i].status==1){
			continue;
		}
		else{
			strcpy((*a)->tmp_reg,r[i].regis);
			r[i].status = 1;
			break;
		}
	}
}

void codegen(AST *ast) {
	static int assigned = 0; //the variable Assigned lhs doesn't need to load
	static int postOperatorStatus = 0;  //if PostInc and PostDec is operation in +-*/=, it sets 1.
	char type[10];
	switch(ast->type){
		case Variable:
			switch(ast->val){
				case 'x':
					strcpy(ast->tmp_reg,"r0");
					r[0].status=1;
					if((x_count ==0 && assigned ==0)||xRegChange ==1){  //first to use x and not left of Assign
						printf("load %s %s\n","r0","[0]");
						x_count++;
						xRegChange =0;
					}
					
					break;
				case 'y':
					strcpy(ast->tmp_reg,"r1");
					r[1].status=1;
					if((y_count ==0 && assigned == 0) ||yRegChange==1){
						printf("load %s %s\n","r1","[4]");
						y_count++;
						yRegChange =0;
					}
					
					break;
				case 'z':
					strcpy(ast->tmp_reg,"r2");
					r[2].status=1;
					if((z_count ==0 && assigned ==0) || zRegChange ==1){
						printf("load %s %s\n","r2","[8]");
						z_count++;
						zRegChange =0;
					}
					
					break;
			}
		case Value:
			break;
		case LPar: //it will replace it's mid information.
			codegen(ast->mid);
			ast->type = ast->mid->type;
			strcpy(ast->tmp_reg,ast->mid->tmp_reg);
			ast->val = ast->mid->val;
			break;
		case RPar:
			break;
		case PostInc:
			postOperatorStatus++; //set for +-*/=
			codegen(ast->mid);
			if(ast->mid->val == 'x'){
				x_PostInc++;
				r[0].status = 2; //while only x++, it will tell system don't forget to +1
			}
			else if( ast->mid ->val == 'y'){
				y_PostInc++;
				r[1].status =2;
			}
			else if (ast->mid->val == 'z'){
				z_PostInc++;
				r[2].status = 2;
			}
			ast->type = Variable;
			strcpy(ast->tmp_reg,ast->mid->tmp_reg); 
			break;
		case PostDec:
			postOperatorStatus++;
			codegen(ast->mid);
			if(ast->mid->val == 'x'){
				x_PostDec++;
				r[0].status = 2; //while only x++, it will tell system don't forget to +1
			}
			else if( ast->mid ->val == 'y'){
				r[1].status =2;
				y_PostDec++;
			}
			else if (ast->mid->val == 'z'){
				z_PostDec++;
				r[2].status = 2;
			}
			strcpy(ast->tmp_reg,ast->mid->tmp_reg);
			if(ast == ast_root){
				printf("add %s %s %d\n",ast->tmp_reg,ast->tmp_reg,1);
			}
			break;
		case PreInc:
			codegen(ast->mid);
			strcpy(ast->tmp_reg,ast->mid->tmp_reg);
			if(ast == ast_root){ //only ++x
				if(ast->val=='x'){
					printf("load %s %s\n","r0","[0]");
				}
				else if(ast->val=='y'){
					printf("load %s %s\n","r1","[4]");
				}
				else{
					printf("load %s %s\n","r2","[8]");
				}
				printf("add %s %s %d\n",ast->tmp_reg,ast->tmp_reg,1);
			}
			else printf("add %s %s %d\n",ast->tmp_reg,ast->tmp_reg,1);
			break;
		case PreDec:
			codegen(ast->mid);
			strcpy(ast->tmp_reg,ast->mid->tmp_reg);
			if(ast == ast_root){
				if(ast->val=='x'){ //only x--
					printf("load %s %s\n","r0","[0]");
				}
				else if(ast->val=='y'){
					printf("load %s %s\n","r1","[4]");
				}
				else{
					printf("load %s %s\n","r2","[8]");
				}
				printf("sub %s %s %d\n",ast->tmp_reg,ast->tmp_reg,1);
			}
			else printf("sub %s %s %d\n",ast->tmp_reg,ast->tmp_reg,1);
			break;
		case Plus: //replace information it's mid
			codegen(ast->mid);
			ast->type = ast->mid->type;
			if(!strcmp(ast->mid->tmp_reg, "r0") || !strcmp(ast->mid->tmp_reg,"r1") || !strcmp(ast->mid->tmp_reg,"r2")){
				strcpy(ast->tmp_reg,ast->mid->tmp_reg);
			}
			else{
				strcpy(ast->tmp_reg,ast->mid->tmp_reg);
			}
			
			break;
		case Minus: 
			codegen(ast->mid);
			ast->type = ast->mid->type;
			if(!strcmp(ast->mid->tmp_reg,"r0") || !strcmp(ast->mid->tmp_reg,"r1") || !strcmp(ast->mid->tmp_reg,"r2")){
				find_reg(&ast);
				printf("sub %s %d %s\n",ast->tmp_reg,0,ast->mid->tmp_reg);
			}
			else{
				strcpy(ast->tmp_reg,ast->mid->tmp_reg);
				if(ast->mid->type == Value){ //(-5)
					ast->mid->val = -(ast->mid->val);
					ast->val = ast->mid->val;
				}
				else if(ast->mid->type == Variable){
					
					printf("sub %s %d %s\n",ast->tmp_reg,0,ast->mid->tmp_reg); //-x
				}
			}
			break;
		case Mul:
			codegen(ast->lhs);
			codegen(ast->rhs);
			if(ast->lhs->type == Value && ast->rhs->type == Value){ //兩邊都是常數
				ast->val = ast->lhs->val * ast->rhs->val;
				ast->type = Value;
			}
			else if(ast->lhs->type == Variable && ast->rhs->type == Variable){ //all variable
				find_reg(&ast);
				printf("mul %s %s %s\n",ast->tmp_reg,ast->lhs->tmp_reg,ast->rhs->tmp_reg);
				ast->type = Variable;
			}
			else if(ast->lhs->type == Value && ast->rhs->type == Variable){  //l is Value and R is Variable
				find_reg(&ast);
				printf("mul %s %s %d\n",ast->tmp_reg,ast->rhs->tmp_reg,ast->lhs->val);
				ast->type = Variable;
			}
			else if(ast->rhs->type == Value && ast->lhs->type == Variable){ //R is Value and l is Variable
				find_reg
				(&ast);
				printf("mul %s %s %d\n",ast->tmp_reg,ast->lhs->tmp_reg,ast->rhs->val);
				ast->type = Variable;
			}
			break;
		case Div:
			codegen(ast->lhs);
			codegen(ast->rhs);
			if(ast->lhs->type == Value && ast->rhs->type == Value){ //都常數
				ast->val = ast->lhs->val / ast->rhs->val;
				ast->type = Value;
			}
			else if(ast->lhs->type == Variable && ast->rhs->type == Variable){ //all variable
				find_reg(&ast);
				printf("div %s %s %s\n",ast->lhs->tmp_reg,ast->lhs->tmp_reg,ast->rhs->tmp_reg);
				ast->type = Variable;
			}
			else if(ast->lhs->type == Value && ast->rhs->type == Variable){ //L Value R Variable
				strcpy(ast->tmp_reg,ast->rhs->tmp_reg);
				printf("div %s %d %s\n",ast->rhs->tmp_reg,ast->lhs->val,ast->rhs->tmp_reg); 
				ast->type = Variable;
			}
			else if(ast->rhs->type == Value && ast->lhs->type == Variable){ //L Variable R Value
				strcpy(ast->tmp_reg,ast->lhs->tmp_reg);
				printf("div %s %s %d\n",ast->tmp_reg,ast->lhs->tmp_reg,ast->rhs->val);
				ast->type = Variable;
			}
			else if ((ast->lhs->type == PostDec || ast->lhs->type == PostInc) && (ast->rhs->type == PostInc || ast->rhs->type ==PreDec)){
				find_reg(&ast);
				printf("div %s %s %s\n",ast->tmp_reg,ast->lhs->mid->tmp_reg,ast->rhs->mid->tmp_reg);
				ast->type = Variable;
			}
			else if((ast->lhs->type == PostDec || ast->lhs->type == PostInc) || (ast->rhs->type == PostInc || ast->rhs->type ==PreDec)){
				if((ast->lhs->type == PostDec || ast->lhs->type == PostInc)){
					find_reg(&ast);
					printf("div %s %s %s\n",ast->tmp_reg,ast->lhs->mid->tmp_reg,ast->rhs->tmp_reg);
					ast->type = Variable;
				}
				else{
					find_reg(&ast);
					printf("div %s %s %s\n",ast->tmp_reg,ast->lhs->tmp_reg,ast->rhs->mid->tmp_reg);
					ast->type = Variable;
				}
			}
			while(postOperatorStatus){ //後序要加1
				postOperatorStatus--;
				if(getOpLevel(ast->lhs->type)==1){
					if(ast->lhs->type == PostInc){
						printf("add %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
						ast->lhs->type = ast->lhs->mid->type;
					}
					else{
						printf("sub %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
						ast->lhs->type = ast->lhs->mid->type;
					}
				}
				if(getOpLevel(ast->rhs->type) ==1){
					if(ast->rhs->type == PostInc){
						printf("add %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
						ast->rhs->type = ast->rhs->mid->type;
					}
					else{
						printf("sub %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
						ast->rhs->type = ast->rhs->mid->type;
					}
				}
			}
			
			
			break;
		case Rem:
			codegen(ast->lhs);
			codegen(ast->rhs);
			if(ast->lhs->type == Value && ast->rhs->type == Value){ //都常數
				ast->val = ast->lhs->val % ast->rhs->val;
				ast->type = Value;			
			}
			else if(ast->lhs->type == Variable && ast->rhs->type == Variable){ //all variable

				find_reg(&ast);
				printf("rem %s %s %s\n",ast->lhs->tmp_reg,ast->lhs->tmp_reg,ast->rhs->tmp_reg);
				ast->type = Variable;
			}
			else if(ast->lhs->type == Value && ast->rhs->type == Variable){ //L Value R Variable 

				find_reg(&ast);
				printf("rem %s %d %s\n",ast->rhs->tmp_reg,ast->lhs->val,ast->rhs->tmp_reg); 
				ast->type = Variable;
			}
			else if(ast->rhs->type == Value && ast->lhs->type == Variable){ //L Vaiable R Value
				find_reg(&ast);
				printf("rem %s %s %d\n",ast->lhs->tmp_reg,ast->lhs->tmp_reg,ast->rhs->val);
				ast->type = Variable;
			}
			while(postOperatorStatus){ //後序要加1
				postOperatorStatus--;
				if(getOpLevel(ast->lhs->type)==1){
					if(ast->lhs->type == PostInc){
						printf("add %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
					}
					else{
						printf("sub %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
					}
				}
				if(getOpLevel(ast->rhs->type) ==1){
					if(ast->lhs->type == PostInc){
						printf("add %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
					}
					else{
						printf("sub %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
					}
				}
			}
			break;
		case Add:
			if(ast->mid){ //Plus
				codegen(ast->mid);
			}
			codegen(ast->lhs);
			codegen(ast->rhs);
			if(ast->lhs->type == Value && ast->rhs->type == Value){  //都常數
				ast->val = ast->lhs->val + ast->rhs->val;
				ast->type = Value;
			}
			else if(ast->lhs->type == Variable && ast->rhs->type == Variable){ //all
				find_reg(&ast);
				printf("add %s %s %s\n",ast->tmp_reg,ast->lhs->tmp_reg,ast->rhs->tmp_reg); 
				ast->type = Variable;
			}
			else if(ast->lhs->type == Value && ast->rhs->type == Variable){ //L VAR R VAL

				find_reg(&ast);
				printf("add %s %d %s\n",ast->tmp_reg,ast->lhs->val,ast->rhs->tmp_reg);
				ast->type = Variable;
			}
			else if(ast->rhs->type == Value && ast->lhs->type == Variable){//L VAL R VAR
				find_reg(&ast);
				printf("add %s %s %d\n",ast->tmp_reg,ast->lhs->tmp_reg,ast->rhs->val);
				ast->type = Variable;
			}
			while(postOperatorStatus){ //後序要加1
				postOperatorStatus--;
				if(getOpLevel(ast->lhs->type)==1){
					if(ast->lhs->type == PostInc){
						printf("add %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
					}
					else{
						printf("sub %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
					}
				}
				if(getOpLevel(ast->rhs->type) ==1){
					if(ast->rhs->type == PostInc){
						printf("add %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
					}
					else{
						printf("sub %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
					}
				}
			}
			break;
		case Sub:
			if(ast->mid){ //Minus
				codegen(ast->mid);
			}
			codegen(ast->lhs);
			codegen(ast->rhs);
			if(ast->lhs->type == Value && ast->rhs->type == Value){  //都常數
				ast->val = ast->lhs->val - ast->rhs->val;
				ast->type = Value;
				
			}
			else if(ast->lhs->type == Variable && ast->rhs->type == Variable){ //all
				find_reg(&ast);
				printf("sub %s %s %s\n",ast->tmp_reg,ast->lhs->tmp_reg,ast->rhs->tmp_reg);
				ast->type = Variable;
			}
			else if(ast->lhs->type == Value && ast->rhs->type == Variable){ //L VAR R VAL
				find_reg(&ast);
				printf("sub %s %d %s\n",ast->rhs->tmp_reg,ast->lhs->val,ast->rhs->tmp_reg);
				ast->type = Variable;
			}
			else if(ast->rhs->type == Value && ast->lhs->type == Variable){ //L VAL R VAR
				find_reg(&ast);
				printf("sub %s %s %d\n",ast->lhs->tmp_reg,ast->lhs->tmp_reg,ast->rhs->val);
				ast->type = Variable;
			}
			while(postOperatorStatus){ //後序要加1
				postOperatorStatus--;
				if(getOpLevel(ast->lhs->type)==1){
					if(ast->lhs->type == PostInc){
						printf("add %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
					}
					else{
						printf("sub %s %s %d\n",ast->lhs->mid->tmp_reg,ast->lhs->mid->tmp_reg,1);
					}
				}
				if(getOpLevel(ast->rhs->type) ==1){
					if(ast->rhs->type == PostInc){
						printf("add %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
					}
					else{
						printf("sub %s %s %d\n",ast->rhs->mid->tmp_reg,ast->rhs->mid->tmp_reg,1);
					}
				}
			}

			break;
		case Assign:
			//store [i] rj
			assigned++;
			codegen(ast->lhs);
			assigned--;
			codegen(ast->rhs);
			if(ast->lhs->type == Variable){
				if((ast->lhs)->val == 'x'){
					strcpy(type,"[0]");
					xRegChange = 1;
				}
				else if((ast->lhs)->val == 'y'){
					strcpy(type,"[4]");
					yRegChange = 1;
				}
				else if((ast->lhs)->val == 'z'){
					strcpy(type,"[8]");
					zRegChange = 1;
				}
			}
			if(ast->rhs->type == Value){
				find_reg(&ast->rhs);
				printf("add %s %d %d\n",ast->rhs->tmp_reg,0,ast->rhs->val);
				printf("store %s %s\n",type,ast->rhs->tmp_reg);
			}
			else if(ast->rhs->type == Variable){
				printf("store %s %s\n",type,ast->rhs->tmp_reg);
			}
			else if(ast->rhs->type == PostDec || ast->rhs->type == PostInc){
				printf("store %s %s\n",type,ast->rhs->mid->tmp_reg);
			}
			
			ast->type = Variable;
			strcpy(ast->tmp_reg,ast->rhs->tmp_reg);
			break;
	}
	// TODO: Implement your own codegen.
	// You may modify the pass parameter(s) or the return type as you wish.
}


void err() {
	puts("Compile Error!");
	exit(0);
}

Token *new_token(int kind, int param) {
	Token *res = (Token*)malloc(sizeof(Token));
	res->kind = kind;
	res->param = param;
	res->prev = res->next = NULL;
	memset(res->tmp_reg,'\0',strlen(res->tmp_reg));
	char tmp[10];
	if(param>='x' && param <='z'){
		sprintf(tmp,"%c",param);
		strcpy(res->tmp_reg,tmp);
	}
	
	return res;
}

AST* new_AST(Token *mid) {
	AST *newN = (AST*)malloc(sizeof(AST));
	newN->lhs = newN->mid = newN->rhs = NULL;
	newN->type = mid->kind;
	newN->val = mid->param;
	if(mid->tmp_reg[0]!='\0'){
		strcpy(newN->tmp_reg,mid->tmp_reg);
	}	
	return newN;
}

int list_to_arr(Token **head) {
	int res = 0;
	Token *now = (*head), *t_head = NULL, *del;
	while(now!=NULL) {
		res++;
		now = now->next;
	}
	now = (*head);
	t_head = (Token*)malloc(sizeof(Token)*res);
	for(int i = 0; i < res; i++) {
		t_head[i] = (*now);
		del = now;
		now = now->next;
		free(del);
	}
	(*head) = t_head;
	return res;
}

int isOp(int x) {
	return Mul <= x && x <= Assign;
}

int isUnary(int x) {
	return PostInc <= x && x <= Minus;
}

int isPar(int x) {
	return LPar <= x && x<= RPar;
}

int isPlusMinus(int x) {
	if(x == Plus) return 1;
	if(x == Minus) return 1;
	return 0;
}

int isOperand(int x) {
	if(x == Value) return 1;
	if(x == Variable) return 1;
	return 0;
}

int getOpLevel(int kind) {
	int res;
	if(kind <= Variable) res = -1;
	else if(kind <= PostDec) res = 1;
	else if(kind <= Minus) res = 2;
	else if(kind <= Rem) res = 3;
	else if(kind <= Sub) res = 4;
	else if(kind <= Assign) res = 14;
	else res = -1;
	return res;
}

int findParPair(Token *arr, int tar, int r) {
	if(arr[tar].kind != LPar) return -1;
	for(int i = tar + 1; i <= r; i++)
		if(arr[i].kind == RPar)
			if(arr[i].param == arr[tar].param)
				return i;
	return -1;
}

int nextSection(Token *arr, int l, int r) {
	int res = l;
	if(arr[l].kind == LPar) {
		res = findParPair(arr, l, r);
		if(res == -1) err();
	}
	return res + 1;
}

int find_Tmid(Token *arr, int l, int r) {
	int big = l;
	for(int i = l; i <= r;) {
		if(getOpLevel(arr[big].kind) <= getOpLevel(arr[i].kind)) {
			if(isPlusMinus(arr[big].kind) && isPlusMinus(arr[i].kind));
			else if(getOpLevel(arr[big].kind) != 14)
				big = i;
		}
		i = nextSection(arr, i, r);
	}
	return big;
}

int var_memory(AST *ast) {
	while(ast->type != Variable)
		ast = ast->mid;
	
	switch(ast->val) {
		case 'x': return 0;
		case 'y': return 4;
		case 'z': return 8;
		default: 
			err();
			return -1;
	}
}

void AST_print(AST *head, int indent) {
	if(head == NULL) return;
	const char kind_only[] = "<%s>\n";
	const char kind_para[] = "<%s>, <%s = %d>\n";
	for(int i=0;i<indent;i++) printf("  ");
	switch(head->type) {
		case LPar:
		case RPar:
		case PostInc:
		case PostDec:
		case PreInc:
		case PreDec:
		case Plus:
		case Minus:
		case Mul:
		case Div:
		case Rem:
		case Add:
		case Sub:
		case Assign:
			printf(kind_only, TYPE[head->type]);
			break;
		case Value:
			printf(kind_para, TYPE[head->type], "value", head->val);
			break;
		case Variable:
			printf(kind_para, TYPE[head->type], "name", head->val);
			break;
		default:
			puts("Undefined AST Type!");
	}
	AST_print(head->lhs, indent+1);
	AST_print(head->mid, indent+1);
	AST_print(head->rhs, indent+1);
}
