all:test_locker

#which compiler
CC=g++

#include which file kept
INCLUDE=../mylib/

#library static
LOCATION=libmy.a

#option for development
CFLAGS=-g

#Mcheck for pragram
MCHECK=valgrind --tool=memcheck 

#-----------------------------------------------------------------------
test_locker:test_locker.o $(INCLUDE)$(LOCATION)
	$(CC) -o test_locker test_locker.o -L$(INCLUDE) -lmy -lpthread

$(INCLUDE)$(LOCATION):locker.o
	ar crs $(LOCATION) locker.o
	mkdir -p $(INCLUDE)
	mv $(LOCATION) $(INCLUDE)

test_locker.o:test_locker.cpp
	$(CC) -c $(CFLAGS) test_locker.cpp -I.

locker.o: locker.cpp
	$(CC) -c $(CFALGS) locker.cpp -I.

#------------------------------------------------------------------------
clean:
	rm -rf *.o && rm -rf ../*.o && rm -rf libst.a && rm -rf test_locker && rm -rf $(INCLUDE)$(INLCUDE)

run:
	$(MCHECK) ./test_locker
