CXXFLAGS=`otawa-config otawa/display --cflags`
LDLIBS=`otawa-config otawa/display --libs`

all: demo binaire

test: demo binaire
	./demo

demo: demo.o
	c++ -o demo demo.o $(LDLIBS)

demo.o: demo.cpp

binaire: binaire.c
	arm-eabi-gcc -nostdlib -nostdinc -static -o binaire binaire.c


graph: demo binaire
	./demo
	dot -Tps _start.dot > cfg.ps


clean:
	rm -f *.o demo binaire *~ *.dot *.ps

.PHONY: all test clean graph
