#ifndef CC_H
#define CC_H
#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef enum {
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_NUM,       // Integer
    ND_LT,        // <
    ND_LE,        // <=
    ND_GT,        // >
    ND_GE,        // >=
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_ASSIGN,    // = 
    ND_LVAR       //ローカル変数
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;

struct Token {
    TokenKind    kind;
    Token       *next;
    int         val;
    int         len;
    char        *str;
};

struct Node {
    NodeKind    kind;
    Node        *lhs;
    Node        *rhs;
    int         val;
    int         offset;
};

struct LVar {
  LVar  *next; // 次の変数かNULL
  char  *name; // 変数の名前
  int   len;    // 名前の長さ
  int   offset; // RBPからのオフセット
};

extern char *user_input;
extern Token *token;
extern Node *code[];
extern struct LVar *locals;


void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *primary();
Node *mul();
Node *unary();
void gen(Node *node);
Node *equality();
Node *relational();
Node *add();
void program();
LVar *find_lvar(Token *tok);

#endif
