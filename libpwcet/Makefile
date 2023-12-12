ARMPREFIX=arm-none-eabi

CC=gcc
ARMCC=$(ARMPREFIX)-gcc
ARMAR=$(ARMPREFIX)-ar
ARMRANLIB=$(ARMPREFIX)-ranlib

CFLAGS=-Ofast -g -Wall -march=native
ARMCFLAGS=-Ofast -g -Wall -fno-tree-loop-distribute-patterns

default: libpwcet.a libpwcet_arm.a
all: clean libpwcet.a libpwcet_arm.a

pwcet.o:
	$(CC) $(CFLAGS) -c pwcet.c -o $@

pwcet_arm.o:
	$(ARMCC) $(ARMCFLAGS) -c pwcet.c -o $@

libpwcet.a: pwcet.o
	ar r $@ $^
	ranlib $@

libpwcet_arm.a: pwcet_arm.o
	$(ARMAR) r $@ $^
	$(ARMRANLIB) $@

clean:
	rm -f *.o *.a
