#include "9cc.h"

Node *code[100];
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
      error(token->str, "'%s'ではありません", op);
  token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error("数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = strlen(str);
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == ';') {
            cur = new_token(TK_RESERVED, cur, strndup(p, 1));
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
            *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == '!') {
            int len = 1;
            if ((p[0] == '<' || p[0] == '>' || p[0] == '=' || p[0] == '!') && p[1] == '=') len = 2;
            cur = new_token(TK_RESERVED, cur, strndup(p, len));
            p += len;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        if (isalpha(*p) || *p == '_') {
            char *start = p;
            while (isalnum(*p) || *p == '_') p++;
            cur = new_token(TK_IDENT, cur, strndup(start, p - start));
            continue;
        }

        error("トークナイズできません: %s", p);
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Token *consume_ident()
{
  if (token->kind != TK_IDENT)
    return 0;
  Token *tok = token;
  token = token->next;
  return tok;
}

Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *primary() {
  if (consume("("))
  {
      Node *node = expr();
      expect(")");
      return node;
  }
  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar)
      node->offset = lvar->offset;
    else
    {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = (!locals ? 0 : locals);
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }
  return new_node_num(expect_number());
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  
  for(;;) {
    if (consume("+"))
       node = new_node(ND_ADD, node,mul());
    else if (consume("-"))
        node = new_node(ND_SUB, node, mul());
    else
        return node;
  }
}

Node *relational() {
  Node *node = add();

  for(;;) {
    if (consume("<"))
        node = new_node(ND_LT, node, add());
    else if (consume("<="))
        node = new_node(ND_LE, node, add());
    else if (consume(">"))
        node = new_node(ND_GT, node, add());
    else if(consume(">="))
        node = new_node(ND_GE, node, add());
    else
        return node;
  }
}

Node *equality() {
  Node *node = relational();

  for(;;) {
    if(consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *assign()
{
  Node *node = equality();

  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *expr()
{
  Node *node = assign();
  return node;
}

Node *stmt()
{
  Node *node = expr();
  expect(";");
  return node;
}

void program()
{
  int i = 0;
  while(!at_eof())
  {
    code[i] = stmt();
    i++;
  }
  code[i] = 0;
}

LVar *find_lvar(Token *tok)
{
  for (LVar *var = locals; var; var = var->next)
  {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }
  return NULL;
}
