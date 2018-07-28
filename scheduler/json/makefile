all:test_json

#which compiler
CC=g++

#option for deveopment
CFLAGS=-g

#Mcheck for program
Mcheck=valgrind --tool=memcheck

#include location kept file
INCLUDE=.

#location for lib
location=../mylib/

#static lib name
MYLIB=libmy.a


#---------------------------------------
test_json:test_json.o $(location)$(MYLIB)
	$(CC) -o test_json test_json.o -L$(location) -lmy

$(location)$(MYLIB):cJSON.o Json.o
	ar -qs $(location)$(MYLIB) cJSON.o Json.o

test_json.o:test_json.cpp
	$(CC) -c $(CFLAGS) test_json.cpp -I$(INCLUDE)

cJSON.o:cJSON.c
	$(CC) -c $(CFLAGS) cJSON.c -I$(INCLUDE)

Json.o:Json.cpp
	$(CC) -c $(CFLAGS) Json.cpp -I$(INCLUDE)

#----------------------------------------
clean:
	rm -rf *.o && rm -rf test_json

run:
	$(Mcheck) ./test_json
