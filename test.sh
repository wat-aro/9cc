#!/bin/bash
success() {
  echo -e "\e[32m$1\e[m"
}

failure() {
  echo -e "\e[31m$1\e[m"
}

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  if [ "$?" != 0 ]; then
    exit 1
  fi
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    success "$input => $actual"
  else
    failure "$input => $expected, but got $actual"
    exit 1
  fi
}

# 加減乗除
assert 21 'int main() { return 5+20-4; }'
assert 21 'int main() { return 5 + 20 - 4; }'
assert 47 'int main() { return 5+6*7;}'
assert 15 'int main() { return 5*(9-6); }'
assert  4 'int main() { return (3+5)/2; }'

# 単行演算子
assert 10 'int main() { return -10+20; }'
assert 10 'int main() { return - -10; }'
assert 10 'int main() { return - - +10; }'

# ==, !=
assert  0 'int main() { return 0==1; }'
assert  1 'int main() { return 42==42; }'
assert  1 'int main() { return 0!=1; }'
assert  0 'int main() { return 42!=42; }'

# <, <=
assert  1 'int main() { return 0<1; }'
assert  0 'int main() { return 1<1; }'
assert  0 'int main() { return 2<1; }'
assert  1 'int main() { return 0<=1; }'
assert  1 'int main() { return 1<=1; }'
assert  0 'int main() { return 2<=1; }'

# >, >=
assert  1 'int main() { return 1>0; }'
assert  0 'int main() { return 1>1; }'
assert  0 'int main() { return 1>2; }'
assert  1 'int main() { return 1>=0; }'
assert  1 'int main() { return 1>=1; }'
assert  0 'int main() { return 1>=2; }'

# statements
assert  3 'int main() { 1; 2; return 3; }'
assert  2 'int main() { int a; a = 2; return a; }'
assert  2 'int main() { int a;a=2;return a; }'
assert 30 'int main() { int a; int b;a = 2 + 3;b = 2 * 3;return a * b; }'
assert 30 'int main() { int foo; int bar; foo = 2 + 3;bar = 2 * 3; return foo * bar; }'
assert  1 'int main() { return 1; }'
assert  6 'int main() { int foo; foo = 2 * 3; return foo; return 5 * 2; }'
assert  3 'int main() { int foo; foo = 2; foo = foo + 1; return foo; }'

# # if
assert 10 'int main() { if (0 == 0) return 10; return 5; }'
assert  5 'int main() { if (0 == 1) return 10; return 5; }'
assert 10 'int main() { if (0 == 0) return 10; else return 1; }'
assert  1 'int main() { if (0 == 1) return 10; else return 1; }'

# # while
assert  2 'int main() { while (0 == 1) return 1; return 2; }'
assert  0 'int main() { int i; i = 10; while (i > 0) i = i - 1; return i; }'

# # for
assert 55 'int main() { int result; result = 0; int i; for (i = 0; i <= 10; i = i + 1) result = result + i; return result; }'
assert  3 'int main() { if (1 == 1) { int a; a = 1; a = a + 2; } return a; }'
assert 55 'int main() { int i; i = 10; int result; result = 0; while (i > 0) { result = result + i; i = i - 1; } return result; }'

# # function
assert 10 'int foo() { return 5 + 5; } int main () { return foo(); }'
assert  8 'int foo(int x) { return x + x; } int main() { return foo(4); }'
assert 11 'int foo(int x, int y) { return x + y; } int main() { return foo(4, 7); }'
assert 21 'int adder(int a, int bb, int ccc, int dddd, int eeeee, int ffffff) { return a + bb + ccc + dddd + eeeee + ffffff; } int main() { return adder(1,2,3,4,5,6); }'

assert  8 'int fib(int n) { if (n <= 0) { return 0; } else if (n == 1) { return 1; } else if (n > 1) { return fib(n - 1) + fib(n - 2);}} int main() { return fib(6); }'
assert 70 'int add(int x, int y, int z) { return x + y + z; } int sub(int x, int y) { return x - y; } int main() { int x; x = add(10, 20, 30); int y; y = sub(20, 10); return x + y;}'

# # *, &
assert 3 'int main() { int x; int *y; x = 3; y = &x; return *y; }'
assert 3 'int main() { int x; int y; int *z; x = 3; y = 5; z = &y + 1; return *z; }'

# pointer type
assert 3 'int main() { int x; int *y; y = &x; *y = 3; return x; }'
assert 3 'int main() { int x; int *y; int z; y = &x; *y = 3; z = 2; return x; }'

# sizeof
assert 4 'int main() { return sizeof(1); }'
assert 4 'int main() { return sizeof(sizeof(1)); }'
assert 8 'int main() { int *x; return sizeof(x); }'

# Array
assert 3 'int main() { int x[10]; *x = 3; return *x; }'
assert 3 'int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }'
assert 3 'int main() { int a[2]; *a = 1; a[1] = 2; int *p; p = a; return *p + *(p + 1); }'
assert 3 'int main() { int a[10]; *a = 1; a[1] = 2; int *p; p = a; return *(p + 1) + *p; }'
assert 63 'int main() { int b[10]; int d; d = 9; b[3] = 7; return b[3] * d; }'
assert 16 'int main() { int b[10]; int d; d = 9; b[3] = 7; return *(b + 3) + d; }'
assert 16 'int main() { int b[10]; int d; d = 9; b[3] = 7; return b[3] + d; }'

# Global variables
assert 5 'int x; int main() { x = 3; return x + 2; }'
assert 7 'int x; int y; int main() { x = 3; y = 4; return x + y; }'
assert 5 'int b[10]; int main() { b[3] = 5; return b[3]; }'
assert 63 'int b[10]; int main() { int d; d = 9; b[3] = 7; return b[3] * d; }'
assert 16 'int b[10]; int main() { int d; d = 9; b[3] = 7; return b[3] + d; }'
assert 8 'int b[10]; int c[2]; int main() { c[1] = 3; int d; d = c[1] * 2; b[d] = 5; return c[1] + b[6];}'

# Char
assert 3 'int main() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }'
assert 5 'char x[3]; int y; int main() { x[0] = -1; x[1] = 2; y = 6; return x[0] + y; }'

echo OK
