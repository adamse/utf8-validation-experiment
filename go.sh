#!/bin/bash

set -o errexit

function r {
    echo "$@"
    "$@"
}

CC=gcc
# CC=clang
CFLAGS=(-march=native -Wall -Werror -g3)
CFLAGS+=(-O2)
# CFLAGS+=(-fsanitize=undefined)
# CFLAGS+=(-fsanitize=address)

function c {
    local out=$1
    shift
    r $CC "${CFLAGS[@]}" -o $out -c "$@"
}

function l {
    local out=$1
    shift
    r $CC "${CFLAGS[@]}" -o $out "$@"
}

c validate.o validate.c
c main.o main.c
l main main.o validate.o

./main "./validateutf8-experiments/examples/twitter.json"
./main "./validateutf8-experiments/examples/hongkong.html"
