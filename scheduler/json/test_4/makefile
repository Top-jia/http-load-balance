all:test_json

#which compiler
CC=g++

#location for file head
location=../

#options for development
CFLAGS=-g

#Mcheck for program
Mcheck= valgrind --tool=memcheck

#dynamic library
DYLIB=libmy.so

#------------------------------------
test_json:test_json.o $(DYLIB)
	$(CC) -o test_json test_json.o  -L. -lmy 

$(DYLIB):$(location)cJSON.c
	$(CC) -fPIC -shared $(CFLAGS) -o $(DYLIB) $(location)cJSON.c -I$(location) 

test_json.o:test_json.cpp
	$(CC) -c $(CFLAGS) test_json.cpp -I$(location)

#------------------------------------
clean:
	rm -rf *.o && rm -rf *.so && rm -rf test_json

run:
	$(MYLIB) ./test_json

