#include "9cc.h"

// 現在着目しているトークン
Token *token;

// ローカル変数
LVar *locals;

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

//
// Parser
//

// 次のトークンが期待している文字列のときには、//
// 真を返す。それ以外の場合には偽を返す。
bool equal(char *op) {
  if (strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを１つ読み勧めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

bool consume_by_token(TokenKind kind) {
  if (token->kind != kind)
    return false;
  token = token->next;
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return false;
  Token *tok = token;
  token = token->next;
  return tok;
}

// 次のトークンが期待している記号のときには、トークンを１つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error_at(token->str, "expected '%s'\n", op);
  }

  token = token->next;
}

// 次のトークンが数値の場合、トークンを１つ読み勧めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

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

Node **program();
Node *function();
Node *compound_stmt();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// program = stmt*
Node **program() {
  Node **code = calloc(100, sizeof(Node));
  int i = 0;
  while (!at_eof()) {
    code[i++] = function();
  }
  code[i] = NULL;

  return code;
}

// args = "(" (ident ("," ident)*)? ")"
Node *args() {
  expect("(");
  Node head = {};
  Node *cur = &head;

  Token *tok = consume_ident();

  if (tok) {
    cur->next = new_node(ND_LVAR, NULL, NULL);
    cur = cur->next;
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals == NULL) {
      lvar->offset = 8;
    } else {
      lvar->offset = locals->offset + 8;
    }
    cur->offset = lvar->offset;
    locals = lvar;

    while (!consume(")")) {
      expect(",");
      tok = consume_ident();
      cur->next = new_node(ND_LVAR, NULL, NULL);
      cur = cur->next;
      LVar *lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      cur->offset = lvar->offset;
      locals = lvar;
    }
  } else {
    expect(")");
  }

  return head.next;
}

// function = ident args compound_stmt
Node *function() {
  locals = NULL;
  Node *node = new_node(ND_FUNCTION, NULL, NULL);
  Token *tok = consume_ident();
  if (!tok)
    error_at(tok->str, "Expected function name\n");
  // Parse function name
  char *func_name = calloc(tok->len, sizeof(char));
  strncpy(func_name, tok->str, tok->len);
  node->name = func_name;

  // Parse arguments
  node->args = args();

  // Parse function bodies
  node->body = compound_stmt()->body;

  node->locals = locals;
  return node;
}

// compound_stmt = "{" stmt* "}"
Node *compound_stmt() {
  expect("{");
  Node head = {};
  Node *cur = &head;
  while (!consume("}"))
    cur = cur->next = stmt();

  Node *node = new_node(ND_BLOCK, NULL, NULL);
  node->body = head.next;
  return node;
}

// stmt = expr ";"
//      | compound_stmt
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt (else stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *stmt() {
  Node *node;

  if (equal("{")) {
    node = compound_stmt();
    return node;
  } else if (consume_by_token(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else if (consume_by_token(TK_IF)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume_by_token(TK_ELSE)) {
      node->els = stmt();
    }
    return node;
  } else if (consume_by_token(TK_WHILE)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    return node;
  } else if (consume_by_token(TK_FOR)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->update = expr();
      expect(")");
    }
    node->body = stmt();
    return node;
  } else {
    node = expr();
  }

  if (!consume(";"))
    error_at(token->str, "';'ではないトークンです");
  return node;
}

// expr = assign
Node *expr() { return assign(); }

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
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

// unary = "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), unary());
  if (consume("*"))
    return new_node(ND_DEREF, unary(), NULL);
  if (consume("&"))
    return new_node(ND_ADDR, unary(), NULL);
  return primary();
}

// primary = num
//         | ident "(" (expr ("," expr)* )? ")"
//         | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    // funcation call
    if (consume("(")) {
      node->kind = ND_FUNCTION_CALL;
      char *str = calloc(tok->len, sizeof(char));
      memcpy(str, tok->str, tok->len);
      node->name = str;

      // arguments
      Node head = {};
      Node *cur = &head;
      if (!consume(")")) {
        cur->next = expr();
        int arg_length = 1;
        while (!consume(")")) {
          expect(",");
          arg_length++;
          if (arg_length > 6)
            error_at(token->str, "引数は6個までです");
          cur = cur->next;
          cur->next = expr();
        }
      }
      node->args = head.next;
    } else { // variable
      node->kind = ND_LVAR;

      LVar *lvar = find_lvar(tok);
      if (lvar) {
        node->offset = lvar->offset;
      } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        if (locals == NULL) {
          lvar->offset = 8;
        } else {
          lvar->offset = locals->offset + 8;
        }
        node->offset = lvar->offset;
        locals = lvar;
      }
    }
    return node;
  }

  return new_node_num(expect_number());
}

Node **parse(Token *tok) {
  token = tok;
  return program();
}
