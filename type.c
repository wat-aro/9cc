#include "9cc.h"

bool is_integer(Type *type) { return type->ty == INT; }

bool is_pointer(Type *type) { return type->ty == PTR; }

Type *pointer_to(Type *ty) {
  Type *type = calloc(1, sizeof(Type));
  type->ty = PTR;
  type->ptr_to = ty;
  return type;
}

Type *type_int = &(Type){INT};

void add_type(Node *node) {
  if (!node || node->type)
    return;
  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->update);
  add_type(node->body);
  add_type(node->next);
  add_type(node->args);

  for (Node *n = node->body; n; n = n->next)
    add_type(n);
  for (Node *n = node->args; n; n = n->next)
    add_type(n);

  switch (node->kind) {
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_ASSIGN:
  case ND_RETURN:
    node->type = node->lhs->type;
    return;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_LVAR:
  case ND_NUM:
    node->type = type_int;
    return;
  case ND_ADDR:
    node->type = pointer_to(node->lhs->type);
    return;
  case ND_DEREF:
    if (node->lhs->type->ty == PTR)
      node->type = node->lhs->type->ptr_to;
    else
      node->type = type_int;
  case ND_FUNCTION_CALL:
    node->type = type_int;
  }
}
