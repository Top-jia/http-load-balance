all:test_json

#which compiler
CC=g++

#where kept file head
INCLUDE=../

#option for Development
CFLAGS=-g

#Mcheck for memory
Mcheck=valgrind --tool=memcheck

#static lib libaray
STLIB=libmyst.a

#dynamic lib library
DYLIB=libmydy.so

#--------------------------------------
test_json:test_json.o $(STLIB) $(DYLIB)
	$(CC) -o test_json test_json.o -L. -lmyst -L. -lmydy

test_json.o:test_json.cpp
	$(CC) -c $(CFLAGS) test_json.cpp -I$(INCLUDE)

$(DYLIB):$(INCLUDE)cJSON.c
	$(CC) -fPIC -shared  $(CFLAGS) -o $(DYLIB) $(INCLUDE)cJSON.c -I$(INCLUDE)

$(STLIB):cJSON.o
	ar -crs $(STLIB) cJSON.o

cJSON.o:$(INCLUDE)cJSON.c
	$(CC) -c $(CFLAGS) $(INCLUDE)cJSON.c -I$(INCLUDE) 

#----------------------------------------
clean:
	rm -rf *.o && rm -rf *.so && rm -rf *.a && rm -rf test_json

run:
	$(Mcheck) ./test_json
