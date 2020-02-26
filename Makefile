CXXFLAGS=`otawa-config otawa/display --cflags`
LDLIBS=`otawa-config otawa/display --libs`
LDLIBS2=`otawa-config otawa/display otawa/cftree --libs`

CXXFLAGS += -std=c++11 -O0 -g -Wall
ARMCC=arm-eabi-gcc

all: dumpcft

test: dumpcft binaire
	./dumpcft ./binaire

dumpcft: dumpcft.o $(HOME)/.otawa/proc/otawa/cftree.so
	$(CXX) -o dumpcft dumpcft.o $(LDLIBS2)

dumpcft.o: dumpcft.cpp

cftree.so: CFTree.cpp include/CFTree.h include/PWCET.h
	$(CXX) -fPIC -shared $(CXXFLAGS) -o cftree.so CFTree.cpp $(LDLIBS)

binaire: binaire.c
	$(ARMCC) -nostdlib -nostdinc -static -o binaire binaire.c

graph: dumpcft binaire
	rm -f *.ps *.dot
	./dumpcft ./binaire
	ls -1 *.dot |while read A ; do dot -Tps $$A > $$A.ps ; done

clean:
	rm -f *.o dumpcft binaire *~ *.dot *.ps *.so *decomp.c

$(HOME)/.otawa/proc/otawa/cftree.so: cftree.so
	mkdir -p $(HOME)/.otawa/proc/otawa
	cp cftree.eld $(HOME)/.otawa/proc/otawa/
	cp cftree.so $(HOME)/.otawa/proc/otawa/

install: $(HOME)/.otawa/proc/otawa/cftree.so

.PHONY: all test clean graph install
