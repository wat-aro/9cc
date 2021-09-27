#include "9cc.h"

extern GVar *globals;

void gen(Node *node);

char *argument_register[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int count = 0;
int jump_count() {
  int i = count;
  count++;
  return i;
}

void gen_lval(Node *node) {
  switch (node->kind) {
  case ND_DEREF:
    gen(node->lhs);
    return;
  case ND_LVAR:
    printf("  lea rax, [rbp - %d]\n", node->offset);
    printf("  push rax\n");
    return;
  case ND_GVAR:
    printf("  lea rax, [rip + %s]\n", node->name);
    printf("  push rax\n");
    return;
  default:
    error("代入の左辺値が変数ではありません");
  }
}

void gen_args(Node *node) {
  if (node) {
    gen_args(node->next);
    gen(node);
  }
}

int locals_size(LVar *locals) {
  int offset = 0;
  for (LVar *var = locals; var; var = var->next) {
    offset = locals->offset;
  }
  return offset;
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
  case ND_GVAR:
    gen_lval(node);
    printf("  pop rax\n");
    if (node->type->ty == INT) {
      printf("  mov eax, [rax]\n");
    } else {
      printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    if (node->lhs->type->ty == PTR) {
      printf("  mov [rax], rdi\n");
    } else {
      printf("  mov [rax], edi\n");
    }
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF:
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    int if_count = jump_count();
    if (node->els) {
      printf("  je  .Lelse%d\n", if_count);
      gen(node->then);
      printf(".Lelse%d:\n", if_count);
      gen(node->els);
    } else {
      printf("  je  .Lend%d\n", if_count);
      gen(node->then);
    }
    printf(".Lend%d:\n", if_count);
    return;
  case ND_FOR: {
    int for_count = jump_count();
    if (node->init) {
      gen(node->init);
    }
    printf(".Lbegin%d:\n", for_count);
    if (node->cond)
      gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", for_count);
    gen(node->body);
    if (node->update)
      gen(node->update);
    printf("  jmp .Lbegin%d\n", for_count);
    printf(".Lend%d:\n", for_count);
    return;
  }
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next) {
      gen(n);
    }
    return;
  case ND_FUNCTION_CALL: {
    gen_args(node->args);
    // ABIに規定されている規定されているレジスタに引数を入れる
    int i = 0;
    for (Node *n = node->args; n; n = n->next) {
      if (i >= 6)
        error("引数は6個までです\n");
      printf("  pop %s\n", argument_register[i]);
      i++;
    }

    printf("  .align 16\n");
    printf("  call %s\n", node->name);
    printf("  push rax\n");
    return;
  }
  case ND_FUNCTION: {
    printf("%s:\n", node->name);
    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");

    if (node->locals) {
      printf("  sub rsp, %d\n", locals_size(node->locals));
    }

    int i = 0;
    for (Node *n = node->args; n; n = n->next) {
      gen_lval(n);
      printf("  push %s\n", argument_register[i]);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      if (n->type->ty == INT) {
        printf("  mov [rax], edi\n");
      } else {
        printf("  mov [rax], rdi\n");
      }
      i++;
    }
    for (Node *n = node->body; n; n = n->next) {
      gen(n);
    }
    return;
  }
  case ND_ADDR:
    gen_lval(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_DECLARE:
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

extern Node *code[100];

void codegen(Node **code) {
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");

  // グローバル変数を出力する
  for (GVar *g = globals; g; g = g->next) {
    printf(".bss\n");
    printf("%s:\n", g->name);
    printf("  .zero %d\n", type_size(g->type));
  }
  // 先頭の式から順にコード生成
  printf(".text\n");
  printf(".globl main\n");
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
  }
}
