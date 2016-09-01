CMD=g++
OPTS=

all: sort

sort: item.o main.o
	${CMD} -o sort ${OPTS} item.o main.o

main.o: main.cpp
	${CMD} -c main.cpp

item.o: item.cpp
	${CMD} -c item.cpp

clean:
	rm -f *.o sort
