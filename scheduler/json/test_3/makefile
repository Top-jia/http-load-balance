all:test_json

#which compiler
CC=g++

#where kept file head
INCLUDE=../

#option for Development
CFLAGS=-g

#Mcheck for program
Mcheck=valgrind --tool=memcheck 

#static library
MYLIB=libjson.a

#-------------------------------------------------
test_json:test_json.o $(MYLIB)
	$(CC) -o test_json test_json.o -L. -ljson

$(MYLIB):cJSON.o
	ar -crs $(MYLIB) cJSON.o

test_json.o:test_json.cpp
	$(CC) -c  $(CFLAGS) test_json.cpp -I$(INCLUDE) 

cJSON.o: $(INCLUDE)cJSON.c
	$(CC) -c  $(CFLAGS) $(INCLUDE)cJSON.c	-I$(INCLUDE)

#------------------------------------------------
run:
	$(Mcheck) ./test_json

clean:
	rm -rf *.o && rm -rf test_json && rm -rf *.a
