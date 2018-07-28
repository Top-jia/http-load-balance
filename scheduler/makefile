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
	$(CC) -o main main.o -L$(location) -lmy

$(location)$(MYLIB):scheduler.o
	mkdir -p $(location)
	(cd log;$(MAKE))
	ar qs $(location)$(MYLIB) scheduler.o
	(cd bstage; $(MAKE))
	(cd sepoll;$(MAKE))
	(cd json;$(MAKE))

main.o:main.cpp 
	$(CC) -c main.cpp $(CFLAGS) -I $(INCLUDE)

scheduler.o:scheduler.cpp
	$(CC) -c scheduler.cpp $(CFLAGS) -I $(INCLUDE)

run:
	$(Mcheck) ./main

clean:
	rm -rf *.o && rm -rf main && rm -rf mylib && rm -rf log_file.txt
	(cd log;$(MAKE) clean)
	(cd bstage;$(MAKE) clean)
	(cd sepoll;$(MAKE) clean)
	(cd json;$(MAKE) clean)
