all: main

main: main.o
	g++ main.o -o sentinel

main.o: src/main.cpp
	g++ -c src/main.cpp

clean:
	rm -rf *.o sentinel
