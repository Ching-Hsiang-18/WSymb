CXXFLAGS=`otawa-config otawa/display --cflags`
LDLIBS=`otawa-config otawa/display --libs`
LDLIBS2=`otawa-config otawa/display otawa/cftree --libs`

CXXFLAGS += -std=c++11

all: demo binaire cftree.so

test: demo binaire
	./demo
	
demo: demo.o
	$(CXX) -o demo demo.o $(LDLIBS2)

demo.o: demo.cpp

cftree.so: CFTree.cpp include/CFTree.h
	$(CXX) -fPIC -shared $(CXXFLAGS) -o cftree.so CFTree.cpp $(LDLIBS)

binaire: binaire.c
	arm-eabi-gcc -nostdlib -nostdinc -static -o binaire binaire.c

graph: demo binaire
	./demo
	dot -Tps _start.dot > cfg.ps

clean:
	rm -f *.o demo binaire *~ *.dot *.ps *.so

install: cftree.so
	mkdir -p $(HOME)/.otawa/proc/otawa
	cp cftree.eld $(HOME)/.otawa/proc/otawa/
	cp cftree.so $(HOME)/.otawa/proc/otawa/
        

.PHONY: all test clean graph
