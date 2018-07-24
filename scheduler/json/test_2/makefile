all:test_json

#which compiler
CC=g++

#where are include kept file
INCLUDE=../

#options for development
CFLAGS=-g

#Mcheck for progarm
Mcheck=valgrind --tool=memcheck

#-----------------------------------
test_json:test_json.o cJSON.o
	$(CC) -o test_json test_json.o cJSON.o

test_json.o:test_json.cpp
	$(CC) -c $(FLAGS) test_json.cpp -I$(INCLUDE)

cJSON.o:$(INCLUDE)cJSON.c
	$(CC) -c $(FLAGS) $(INCLUDE)cJSON.c -I$(INCLUDE)

clean:
	rm -rf *.o && rm -rf test_json && rm -rf *.txt

run:
	$(Mcheck) ./test_json
