@echo off
mkdir build 2> NUL & gcc -Wall -pedantic json.c murmurhash.c -o .\build\json.exe
