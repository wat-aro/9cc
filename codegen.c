#include "9cc.h"

void gen(Node *node);

int count = 0;
int jump_count() {
  int i = count;
  count++;
  return i;
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_args(Node *node) {
  if (node) {
    gen_args(node->next);
    gen(node);
  }
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
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
    for (Node *n = node->body; n; n = n->next)
      gen(n);
    return;
  case ND_FUNCTION_CALL:
    gen_args(node->args);
    // ABIに規定されている規定されているレジスタに引数を入れる
    int i = 1;
    for (Node *n = node->args; n; n = n->next) {
      if (i == 1) {
        printf("  pop rdi\n"); // 第1引数
      } else if (i == 2) {
        printf("  pop rsi\n"); // 第2引数
      } else if (i == 3) {
        printf("  pop rdx\n");
      } else if (i == 4) {
        printf("  pop rcx\n");
      } else if (i == 5) {
        printf("  pop r8\n");
      } else if (i == 6) {
        printf("  pop r9\n");
      } else {
        error("引数は6個までです");
      }
      i++;
    }
    printf("  call %s\n", node->name);
    return;
  }

  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
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
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ
  // 変数26個分の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
