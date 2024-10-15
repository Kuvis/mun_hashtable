CC = gcc

.PHONY: all test str_example int_example

all: test int_example str_example int_example_typesafe str_example_typesafe

test: test.c ../hashtable.h ../hashtable.c
	$(CC) -Wall -O3 -flto test.c ../hashtable.c -o test
#$(CC) -Wall -O0 -g test.c ../hashtable.c -o test

int_example: int_example.c ../hashtable.h ../hashtable.c
	$(CC) -Wall -O0 -fsanitize=address int_example.c ../hashtable.c -o int_example

str_example: str_example.c ../hashtable.h ../hashtable.c
	$(CC) -Wall -O0 -fsanitize=address str_example.c ../hashtable.c -o str_example

int_example_typesafe: int_example_typesafe.c ../hashtable.h ../hashtable.c
	$(CC) -Wall -O0 -fsanitize=address int_example_typesafe.c ../hashtable.c -o \
	int_example_typesafe

str_example_typesafe: str_example_typesafe.c ../hashtable.h ../hashtable.c
	$(CC) -Wall -O0 -fsanitize=address str_example_typesafe.c ../hashtable.c -o \
	str_example_typesafe
