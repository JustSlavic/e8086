#!/bin/bash

C_FLAGS="-std=c11"
WARNINGS="-Wall -Werror"
DEFINES=""

INCLUDES="-I../code"

gcc $C_FLAGS $WARNINGS $DEFINES $INCLUDES -o e8086 ../code/main.c
