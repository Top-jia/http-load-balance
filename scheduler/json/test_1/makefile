all: test_cjson

#which compiler
CC=g++

#option for development
CFLAGS=-g

#Mcheck for program
Mcheck=valgrind --tool=memcheck

#include kept head location
location=../

#-----------------------------------------
test_cjson:test_cjson.o cJSON.o
	$(CC) -o test_cjson test_cjson.o cJSON.o

test_cjson.o:test_cjson.cpp
	$(CC) -c test_cjson.cpp  $(CFLAGS) -I$(location)

cJSON.o: $(location)cJSON.c
	$(CC) -c $(location)cJSON.c $(CFLAGS) -I$(location)

run:
	$(Mcheck) ./test_cjson

clean:
	rm -rf *.o && rm -rf test_cjson
