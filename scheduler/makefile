all:main

#which compiler
CC=g++

#static location
location=./mylib/

#options for develpment
CFLAGS=-g

#static library
MYLIB=libmy.a

#Mcheck for program
Mcheck=valgrind --tool=memcheck

#where kept file location
INCLUDE=.

#--------------------------------------

main:main.o $(location)$(MYLIB)
	$(CC) -o main main.o -L$(location) -lmy -lpthread

$(location)$(MYLIB):scheduler.o
	(cd locker;$(MAKE))
	(cd log;$(MAKE))
	(cd bstage; $(MAKE))
	(cd sepoll;$(MAKE))
	(cd json;$(MAKE))
	(cd threads;$(MAKE))
	(cd fork;$(MAKE))
	ar qs $(location)$(MYLIB) scheduler.o

main.o:main.cpp 
	$(CC) -c main.cpp $(CFLAGS) -I $(INCLUDE) 

scheduler.o:scheduler.cpp
	$(CC) -c scheduler.cpp $(CFLAGS) -I $(INCLUDE)

run:
	$(Mcheck) ./main

clean:
	rm -rf *.o && rm -rf main && rm -rf mylib && rm -rf ./etc/log_file.txt
	(cd locker;$(MAKE) clean)
	(cd log;$(MAKE) clean)
	(cd bstage;$(MAKE) clean)
	(cd sepoll;$(MAKE) clean)
	(cd json;$(MAKE) clean)
	(cd threads;$(MAKE) clean)
