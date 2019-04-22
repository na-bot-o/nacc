#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//token type value
enum {
  TK_NUM = 256, //intefer token
  TK_EOF,       // end of input token
};

//token type

typedef struct {
  int ty; //token type
  int val; //value if ty is TK_NUM
  char *input; // token string (error message)
} Token;

// save tokenized string to this array
// limit is 100
Token tokens[100];

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

int main(int argc, char **argv) {
  if (argc != 2){
    fprintf(stderr,"引数の個数が正しくありません\n");
    return 1;
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
