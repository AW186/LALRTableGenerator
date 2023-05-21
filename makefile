all: test

test: main.o syntaxparser.o lrgen.o
	g++ -o test main.o syntaxparser.o lrgen.o

lrgen.o: lrgen.cpp lrgen.hpp syntaxparser.hpp reader.hpp

main.o: main.cpp syntaxparser.hpp reader.hpp
	g++ -c main.cpp

syntaxparser.o: syntaxparser.cpp syntaxparser.hpp reader.hpp
	g++ -c syntaxparser.cpp

testcase:
	gcc -E syntax.c -o syntax.lr

clear:
	rm *.o
	rm test