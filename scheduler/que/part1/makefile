all:  test_show

#which compiler
CC=g++

#where are include files kept
INCLUDE=.

#Options for development
CFLAGS=-g

#Mcheck for process
Mcheck=valgrind --tool=memcheck 

#Debug for process
Debug=gdb 

#Static library
Static_lib=libmystatic.a

#library location
location=.

#Dynamic library
Dynamic_lib=libmydynamic.so

#Subdir static library
Subdir_lib=libtest.a

#-------------------------------------------
test_show: init test_show.o $(Static_lib) $(Dynamic_lib) $(Subdir_lib)
	#self 本身生成
	$(CC) -o test_self	test_show.o test_a.o test_b.o
	#static 静态生成
	$(CC) -o test_static	test_show.o -L$(location)/lib	-lmystatic
	#dynamic 动态生成
	$(CC) -o test_dynamic	test_show.o -L$(location)/lib	-lmydynamic
	#subdir static 子目录的静态库生成
	$(CC) -o test_subdir	test_show.o -L$(location)/lib	-ltest

init:
	mkdir -p $(location)/lib

$(Static_lib):test_a.o test_b.o
	ar	-crs	$(Static_lib)	test_a.o test_b.o
	mv	$(Static_lib)	$(location)/lib

$(Subdir_lib):test_a.o test_b.o
	(cd makedir; $(MAKE))
	ar -qs $(location)/lib/libtest.a test_a.o test_b.o


$(Dynamic_lib):test_a.cpp test_b.cpp
	$(CC) -fPIC -shared -o $(Dynamic_lib) test_a.cpp test_b.cpp
	mv $(Dynamic_lib) $(location)/lib
	export LD_LIBRARY_PATH=$(location)/lib:

test_show.o:test_show.cpp
	$(CC) -c $(CFLAGS)  test_show.cpp

test_a.o:test_a.cpp
	$(CC) -c $(CFLAGS)  test_a.cpp

test_b.o:test_b.cpp
	$(CC) -c $(CFLAGS)	test_b.cpp

#-------------------------------------------
debug: test_show
	$(Debug) test_show

mcheck: test_show
	$(Mcheck) ./test_show

run:
	./test_show

clean:
	rm -rf *.o && rm -rf test_show && rm -rf lib && rm -rf test_dynamic \
		&& rm -rf test_self && rm -rf test_static && rm -rf test_subdir
	(cd makedir; $(MAKE) clean)

