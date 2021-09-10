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
assert 21 'main() { return 5+20-4; }'
assert 21 'main() { return 5 + 20 - 4; }'
assert 47 'main() { return 5+6*7;}'
assert 15 'main() { return 5*(9-6); }'
assert  4 'main() { return (3+5)/2; }'

# 単行演算子
assert 10 'main() { return -10+20; }'
assert 10 'main() { return - -10; }'
assert 10 'main() { return - - +10; }'

# ==, !=
assert  0 'main() { return 0==1; }'
assert  1 'main() { return 42==42; }'
assert  1 'main() { return 0!=1; }'
assert  0 'main() { return 42!=42; }'

# <, <=
assert  1 'main() { return 0<1; }'
assert  0 'main() { return 1<1; }'
assert  0 'main() { return 2<1; }'
assert  1 'main() { return 0<=1; }'
assert  1 'main() { return 1<=1; }'
assert  0 'main() { return 2<=1; }'

# >, >=
assert  1 'main() { return 1>0; }'
assert  0 'main() { return 1>1; }'
assert  0 'main() { return 1>2; }'
assert  1 'main() { return 1>=0; }'
assert  1 'main() { return 1>=1; }'
assert  0 'main() { return 1>=2; }'

# statements
assert  3 'main() { 1; 2; return 3; }'
assert  2 'main() { a = 2; return a; }'
assert  2 'main() { a=2;return a; }'
assert 30 'main() { a = 2 + 3;b = 2 * 3;return a * b; }'
assert 30 'main() { foo = 2 + 3;bar = 2 * 3; return foo * bar; }'
assert  1 'main() { return 1; }'
assert  6 'main() { foo = 2 * 3; return foo; return 5 * 2; }'
assert  3 'main() { foo = 2; foo = foo + 1; return foo; }'

# if
assert 10 'main() { if (0 == 0) return 10; return 5; }'
assert  5 'main() { if (0 == 1) return 10; return 5; }'
assert 10 'main() { if (0 == 0) return 10; else return 1; }'
assert  1 'main() { if (0 == 1) return 10; else return 1; }'

# while
assert  2 'main() { while (0 == 1) return 1; return 2; }'
assert  0 'main() { i = 10; while (i > 0) i = i - 1; return i; }'

# for
assert 55 'main() { result = 0; for (i = 0; i <= 10; i = i + 1) result = result + i; return result; }'
assert  3 'main() { if (1 == 1) { a = 1; a = a + 2; } return a; }'
assert 55 'main() { i = 10; result = 0; while (i > 0) { result = result + i; i = i - 1; } return result; }'

# function
assert 10 'foo() { return 5 + 5; } main () { return foo(); }'
assert  8 'foo(x) { return x + x; } main() { return foo(4); }'
assert 11 'foo(x, y) { return x + y; } main() { return foo(4, 7); }'
assert 21 'adder(a, bb, ccc, dddd, eeeee, ffffff) { return a + bb + ccc + dddd + eeeee + ffffff; } main() { return adder(1,2,3,4,5,6); }'

assert  8 'fib(n) { if (n <= 0) { return 0; } else if (n == 1) { return 1; } if (n > 1) { return fib(n - 1) + fib(n - 2);}} main() { return fib(6); }'

echo OK
