all:test_locker

#which compiler
CC=g++

#include which file kept
INCLUDE=../

#library static
LOCATION=libst.a

#option for development
CFLAGS=-g

#Mcheck for pragram
MCHECK=valgrind --tool=memcheck 

#-----------------------------------------------------------------------
test_locker:test_locker.o $(LOCATION)
	$(CC) -o test_locker test_locker.o -L. -lst -lpthread

$(LOCATION):locker.o
	ar -rcs $(LOCATION) locker.o

test_locker.o:test_locker.cpp
	$(CC) -c $(CFLAGS) test_locker.cpp -I$(INCLUDE) 

locker.o:$(INCLUDE)locker.cpp
	$(CC) -c $(CFALGS) $(INCLUDE)locker.cpp -I$(INCLUDE)

#------------------------------------------------------------------------
clean:
	rm -rf *.o && rm -rf ../*.o && rm -rf libst.a && rm -rf test_thread

run:
	$(MCHECK) ./test_thread
