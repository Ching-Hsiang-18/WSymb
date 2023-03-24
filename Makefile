CXXFLAGS=`otawa-config otawa/etime otawa/display --cflags`
CXXFLAGS+=-g -Ofast -march=native
LDFLAGS=-g
LDLIBS=`otawa-config otawa/etime otawa/display --libs`
LDLIBS2=`otawa-config otawa/icat3 otawa/etime otawa/display otawa/cftree --libs`
CFLAGS=-Ofast -march=native -g -Wall -W -fsanitize=address

CXXFLAGS += -std=c++11 -g -Wall

default: dumpcft pwcet/lib/libpwcet-runtime.a
all: clean uninstall dumpcft pwcet/lib/libpwcet-runtime.a

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

uninstall:
	rm -f $(HOME)/.otawa/proc/otawa/cftree.eld
	rm -f $(HOME)/.otawa/proc/otawa/cftree.so

.PHONY: all test clean graph install
