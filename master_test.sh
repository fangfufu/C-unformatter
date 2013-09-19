#!/bin/bash
cp unformatter.c test.c &&\
cc test.c -o test &&\
./test test.c &&\
cc test.c -o test &&\
./test test.c &&\
cc test.c -o test &&\
./test test.c &&\
cp unformatter.c test.c &&\
cc test.c -o test &&\
./test -c test.c &&\
cc test.c -o test &&\
./test -c test.c &&\
cc test.c -o test &&\
./test -c test.c &&\
rm test*
