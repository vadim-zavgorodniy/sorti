CMD=g++

all: sort

sort: main.o
	${CMD} -o sort main.o

main.o: main.cpp
	${CMD} -c main.cpp

clean:
	rm -f *.o sort
