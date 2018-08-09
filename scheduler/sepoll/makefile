all:test_sepoll

#where compiler
CC=g++

#where are include kept file
INCLUDE=.

#debug for development
CFLAGS=-g

#Mcheck for process
Mcheck=valgrind --tool=memcheck

#location library
LOCATION=../mylib/

#static library
MYLIB=libmy.a

#-----------------------------------------
test_sepoll:test_sepoll.o $(LOCATION)$(MYLIB)
	$(CC) -o test_sepoll test_sepoll.o -L$(LOCATION) -lmy -lpthread

$(LOCATION)$(MYLIB):sepoll.o
	ar qs $(LOCATION)$(MYLIB) sepoll.o

test_sepoll.o:test_sepoll.cpp
	$(CC) -c test_sepoll.cpp -I $(CFLAGS)

sepoll.o:sepoll.cpp
	$(CC) -c sepoll.cpp -I $(CFLAGS)


#-------------------------------------------
clean:
	rm -rf *.o && rm -rf test_sepoll && rm -rf log_file.txt

run:
	$(Mcheck) ./test_sepoll
