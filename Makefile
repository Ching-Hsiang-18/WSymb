CXXFLAGS=`otawa-config otawa/display --cflags`
CXXFLAGS+=-g
LDFLAGS=-g
LDLIBS=`otawa-config otawa/display --libs`
LDLIBS2=`otawa-config otawa/display otawa/cftree --libs`
CFLAGS=-O0 -g -Wall -W -fsanitize=address

CXXFLAGS += -std=c++11 -O0 -g -Wall

all: dumpcft pwcet/lib/libpwcet-runtime.a

dumpcft: dumpcft.o $(HOME)/.otawa/proc/otawa/cftree.so
	$(CXX) $(LDFLAGS) -o dumpcft dumpcft.o $(LDLIBS2)

dumpcft.o: dumpcft.cpp

cftree.so: CFTree.cpp PWCET.c include/CFTree.h include/PWCET.h pwcet/include/pwcet-runtime.h
	$(CXX) -fPIC -shared $(CXXFLAGS) -o cftree.so PWCET.c CFTree.cpp $(LDLIBS)

pwcet/lib/libpwcet-runtime.a: PWCET.c include/PWCET.h pwcet/include/pwcet-runtime.h
	$(CC) $(CFLAGS) -c PWCET.c
	ar r pwcet/lib/libpwcet-runtime.a PWCET.o
	ranlib pwcet/lib/libpwcet-runtime.a

clean:
	rm -f *.o dumpcft *~ *.dot *.ps *.so *decomp.c *.gch pwcet/lib/*.a

$(HOME)/.otawa/proc/otawa/cftree.so: cftree.so
	mkdir -p $(HOME)/.otawa/proc/otawa
	cp cftree.eld $(HOME)/.otawa/proc/otawa/
	cp cftree.so $(HOME)/.otawa/proc/otawa/

install: pwcet/lib/libpwcet-runtime.a dumpcft $(HOME)/.otawa/proc/otawa/cftree.so

.PHONY: all test clean graph install
