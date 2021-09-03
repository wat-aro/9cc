#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("invalid number of arguments\n", argv[0]);
    return 1;
  }

  Token *token = tokenize(argv[1]);
  Node **code = parse(token);
  codegen(code);

  return 0;
}
