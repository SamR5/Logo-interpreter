all: flocking_simulation

flocking_simulation: main.o custom_exception.o file_parse.o
	g++ -o Logo_interpreter main.o custom_exception.o file_parse.o -lGL -lGLU -lglut -O3 -no-pie

custom_exception.o: custom_exception.cpp
	g++ -o custom_exception.o -c custom_exception.cpp -std=c++11 -O3

file_parse.o: file_parse.cpp
	g++ -o file_parse.o -c file_parse.cpp -std=c++11 -O3

main.o: main.cpp
	g++ -o main.o -c main.cpp -std=c++11 -O3

debug: main.o custom_exception.o file_parse.o
	g++ -o Logo_interpreter_db main.o custom_exception.o file_parse.o -lGL -lGLU -lglut -g
	gdb ./Logo_interpreter_db
	rm -f Logo_interpreter_db core

clean:
	rm -f *.o core
