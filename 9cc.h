#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
  TK_RETURN,   // return
  TK_IF,       // if
  TK_ELSE,     // else
  TK_WHILE,    // while
  TK_FOR,      // for
  TK_SIZEOF,   // sizeof
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

struct Type {
  enum { VOID, INT, PTR, ARRAY } ty;
  int size;
  struct Type *ptr_to;
  size_t array_size;
};

typedef struct Type Type;

// ローカル変数の型
typedef struct LVar LVar;
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
  Type *type;
};

typedef struct GVar GVar;
struct GVar {
  GVar *next;
  char *name;
  int len;
  Type *type;
};

typedef struct Token Token;

Token *tokenize(char *p);

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

typedef enum {
  ND_ADD,           // +
  ND_SUB,           // -
  ND_MUL,           // *
  ND_DIV,           // /
  ND_EQ,            // ==
  ND_NE,            // !=
  ND_LT,            // <
  ND_LE,            // <=
  ND_ASSIGN,        // =
  ND_LVAR,          // ローカル変数
  ND_GVAR,          // グローバル変数
  ND_NUM,           // integer
  ND_RETURN,        // return
  ND_IF,            // if
  ND_FOR,           // for
  ND_BLOCK,         // {}
  ND_FUNCTION_CALL, // call function
  ND_FUNCTION,      // function
  ND_ADDR,          // & address
  ND_DEREF,         // * dereference
  ND_DECLARE,       // int a;
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;    // kindがND_NUMの場合のみ使う
  int offset; // kindがND_LVARの場合のみ使う
  // if
  Node *cond;
  Node *then;
  Node *els;

  // for
  Node *init;
  Node *update;
  Node *body;

  // block
  Node *next;

  // function call
  char *name;
  Node *args;
  LVar *locals;
  Type *type; // 変数の型
};

Node **parse(Token *tok);

void codegen();

// type

bool is_integer(Type *type);
bool is_pointer(Type *type);
bool is_array(Type *type);
Type *pointer_to(Type *ty);
Type *array_of(Type *ty, int size);
void add_type(Node *node);

extern Type *type_int;
