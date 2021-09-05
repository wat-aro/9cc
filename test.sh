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

assert 21 '5+20-4;'
assert 21 '5 + 20 - 4;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'
assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'
assert 3 '1; 2; 3;'
assert 2 'a = 2;'
assert 2 'a=2;a;'
assert 30 'a = 2 + 3;b = 2 * 3;a * b;'
assert 30 'foo = 2 + 3;bar = 2 * 3;foo * bar;'
assert 1 'return 1;'
assert 6 'foo = 2 * 3; return foo; return 5 * 2;'
assert 3 'foo = 2; foo = foo + 1; foo;'
assert 10 'if (0 == 0) return 10; return 5;'
assert 5 'if (0 == 1) return 10; return 5;'
assert 10 'if (0 == 0) return 10; else return 100;'
assert 100 'if (0 == 1) return 10; else return 100;'
assert 2 'while (0 == 1) return 1; return 2;'
assert 0 'i = 10; while (i > 0) i = i - 1; return i;'
assert 55 'result = 0; for (i = 0; i <= 10; i = i + 1) result = result + i; return result;'
assert 3 'if (1 == 1) { a = 1; a = a + 2; } return a;'
assert 55 'i = 10; result = 0; while (i > 0) { result = result + i; i = i - 1; } return result;'

echo OK
