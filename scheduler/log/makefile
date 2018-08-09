all:test_log

#whilch compiler
CC=g++

#where are include file kept
INCLUDE=.

#options for develoption
CFLAGS=-g

#local libraries
MYLIB=libmy.a

#library location
LOCATION=../mylib/

#Mcheck
Mcheck=vargrind --tool=memcheck

test_log:test_log.o $(LOCATION)$(MYLIB)
	$(CC) -o test_log test_log.o -L$(LOCATION)  -lmy -lpthread

$(LOCATION)$(MYLIB):log.o
	ar -qs $(LOCATION)$(MYLIB) log.o

test_log.o:test_log.cpp
	$(CC) -c  $(CFLAGS) test_log.cpp

log.o:log.cpp
	$(CC) -c  $(CFLAGS) log.cpp

clean:
	rm -rf *.o && rm -rf test_log &&rm -rf *.a && rm -rf *.txt && rm -rf ../etc/log_file.txt

run:
	$(Mcheck) ./test_log
