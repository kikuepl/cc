#ifndef MYCOMPILER_H
#define MYCOMPILER_H

#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

extern char *user_input;
extern Token *token;

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *primary();
Node *mul();
void gen(Node *node);

#endif