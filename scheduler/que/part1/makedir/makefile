all: test_cshow

#which compiler
CC=g++

#where are include kept file
INCLUDE=.

#option for develpment
CFLAGS=-g

#Static library
Static_lib=libtest.a

#-----------------------------------
test_cshow:test_cshow.o $(Static_lib)
	$(CC) -o test_cshow test_cshow.o -L$(INCLUDE) -ltest
	cp $(Static_lib) ../lib

test_cshow.o:test_cshow.cpp
	$(CC) -c $(CFLAGS) test_cshow.cpp

$(Static_lib):test_c.o
	ar -crs	$(Static_lib)	test_c.o

c_test_o:test_c.cpp
	$(CC) -c $(CFLAGS)	test_c.cpp

run:
	./test_cshow

clean:
	rm -rf test_cshow && rm -rf *.o && rm -rf $(Static_lib)
