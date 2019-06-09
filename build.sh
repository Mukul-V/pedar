#!/bin/sh
set -ex
gcc -Wall -O2 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -ggdb -fno-omit-frame-pointer -g utils.c table.c array.c lexer.c parser.c eval.c memory.c vm.c -o pedar
