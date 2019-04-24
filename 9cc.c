#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>



//token type value
enum {
  TK_NUM = 256, //intefer token
  TK_EOF,       // end of input token
  ND_NUM = 256, //int node type
};



//token type

typedef struct {
  int ty; //token type
  int val; //value if ty is TK_NUM
  char *input; // token string (error message)
} Token;

typedef struct Node {
  int ty;   //operator or ND_NUM
  struct Node *lhs; //left side
  struct Node *rhs; //right side
  int val;  //use if ty is ND_NUM
} Node;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;


// save tokenized string to this array
// limit is 100
Token tokens[100];

//prototype 
Node *term();
Node *add();
Node *mul();



Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node -> ty = ty;
  node -> lhs = lhs;
  node -> rhs = rhs;
  return node;
}


Node *new_node_num(int val){
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Vector *new_vector(){
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem){
  if(vec->capacity == vec->len){
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

//token index
int pos = 0;

int consume(int ty) {
  if(tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *term(){
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      error("not close brackets: %s",tokens[pos].input);
    return node;
  }

  if(tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);
  error("this token isn't int or open brackets: %s",tokens[pos].input);
}

Node *mul() {
  Node *node = term();

  for(;;) {
    if(consume('*'))
      node = new_node('*',node,term());
    else if (consume('/'))
      node = new_node('/',node, term());
    else
      return node;
  }
}

Node *add(){
  Node *node = mul();
 
  for(;;) {
    if(consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-',node,mul());
    else
      return node;
  }
}



void gen(Node *node){
  if(node->ty == ND_NUM) {
    printf("  push %d\n",node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->ty){
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
  }
}

//error func
//take arg the same as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);

}

//save string indicated as p allocated by token
void tokenize(char *p){
  int i = 0;
  while(*p) {
  //skip empty
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+' || *p == '-'){
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }
    
    if(isdigit(*p)){
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }
    error("can't tokenize: %s",p);
    exit(1);
  }
  
  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}


//test code
int expect(int line, int expected, int actual){
  if(expected == actual)
    return;
  fprintf(stderr, "%d: %d expected, but got %d\n",line, expected,actual);
  exit(1);
}

void runtest(){
  Vector *vec = new_vector();
  expect(__LINE__,0, vec->len);

  for(int i = 0; i < 100; i++)
    vec_push(vec, (void *)i);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (int)vec->data[0]);
    expect(__LINE__, 50, (int)vec->data[50]);
    expect(__LINE__, 90, (int)vec->data[90]);

    printf("OK\n");
}


int main(int argc, char **argv) {
  if (argc != 2){
    fprintf(stderr,"引数の個数が正しくありません\n");
    return 1;
  }
  
  if(strcmp(argv[1],"-test")==0){
    printf("run test");
    runtest();
    return 1;
    printf("end test");
  }

   tokenize(argv[1]);

//output assembly the first half
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");


  //check output first mov instruction because head of expression is num
  if(tokens[0].ty != TK_NUM){
    error("first expression isn't num");
  }

  printf("  mov rax, %d\n", tokens[0].val);

  //output assembly consumer + <num> or - <num>
  int i = 1;

  while(tokens[i].ty != TK_EOF) {
    if(tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM){
        error("unexpected token: %s",tokens[i].input);
      }
      printf("    add rax, %d\n",tokens[i].val);
      i++;
      continue;
    }
    if(tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM){
        error("unexpected token: %s",tokens[i].input);
      }
      printf("    sub rax, %d\n",tokens[i].val);
      i++;
      continue;
    }

   
    error("unexpected character: %s",tokens[i].input);
  }

  printf("  ret\n");
  return 0;

}

