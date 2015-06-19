
all:
	g++ -Wall -c main.cc
	g++ -Wall -fpic -c foo.cc
	g++ -Wall -shared -o libfoo.so foo.o
	g++ -Wall -o test main.o -L. -lfoo

clean:
	rm -f *.o test *.so
