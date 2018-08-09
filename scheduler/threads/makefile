all:test_threadpool

#which compiler
CC=g++

#option for development
CFLAGS=-g

#static library
MYLIB=libmy.a

#static location
location=../mylib/

#Mcheck for program
Mcheck=valgrind --tool=memcheck 

#where kept file head
INCLUDE=.

#----------------------------------------

test_threadpool:test_threadpool.o $(location)$(MYLIB) 
	$(CC) -o test_threadpool test_threadpool.o -L$(location) -lmy -lpthread -I$(INCLUDE)

test_threadpool.o:test_threadpool.cpp
	$(CC) -c $(CFLAGS) test_threadpool.cpp

$(location)$(MYLIB):threadpool.o 
	ar -qs $(location)$(MYLIB) threadpool.o 

threadpool.o:threadpool.cpp
	$(CC) -c $(CFLAGS) threadpool.cpp -I$(INCLUDE)

#-----------------------------------------
clean:
	rm -rf *.o && rm -rf test_threadpool && rm -rf log_file.txt

run:
	$(Mcheck) ./test_threadpool
