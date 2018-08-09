all:test_bstage

#Which comapiler
CC=g++

#where are include file kept
INCLUDE=.

#option for develoption
CFLAGS=-g

#Mcheck for program
Mcheck=valgrind --tool=memcheck

#location library
location=../mylib/

#static library
MYLIB=libmy.a

#--------------------------------
test_bstage:test_bstage.o  $(location)$(MYLIB)
	$(CC) -o test_bstage test_bstage.o -L$(location) -lmy -lpthread

$(location)$(MYLIB):bstage.o
	ar -qs $(location)$(MYLIB) bstage.o

test_bstage.o:test_bstage.cpp 
	$(CC) -c test_bstage.cpp -I  $(INCLUDE) $(CFLAGS)

bstage.o:bstage.cpp
	$(CC) -c bstage.cpp -I $(INCLUDE) $(CFLAGS)

clean:
	rm -rf *.o && rm -rf test_bstage

run:
	$(Mcheck) ./test_bstage
