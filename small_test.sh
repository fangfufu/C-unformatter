#!/bin/bash
cp unformatter.c test.c &&\
./unformatter test.c &&\
nano test.c
