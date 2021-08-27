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

assert 21 5+20-4
assert 21 "5 + 20 - 4"
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'

echo OK
