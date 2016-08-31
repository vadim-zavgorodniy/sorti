CMD=g++

all: generate

generate: main.o
	${CMD} -o generate main.o

main.o: main.cpp
	${CMD} -c main.cpp

clean:
	rm -f *.o generate
