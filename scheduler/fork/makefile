all:server

#whilch compiler
CC=g++

#where are include file kept
INCLUDE=.

#options for develoption
CFLAGS=-g

#Mcheck
Mcheck=vargrind --tool=memcheck

server.o:server.cpp
	$(CC) -c  $(CFLAGS) server.cpp

server:server.o
	$(CC) -o server server.o

clean:
	rm -rf *.o && rm -rf server
