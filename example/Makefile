ARMCC=arm-unknown-linux-gnueabi-gcc
CFLAGS=-Wall -W -fsanitize=address
LDLIBS=../pwcet/lib/libpwcet-runtime.a
LDFLAGS=-fsanitize=address

all: pwcet_instantiator

pwcet_instantiator: pwcet_instantiator.c example-pwcet.h ../pwcet/include/pwcet-runtime.h ../pwcet/lib/libpwcet-runtime.a
	cc $(CFLAGS) -c pwcet_instantiator.c
	cc $(LDFLAGS) -o pwcet_instantiator pwcet_instantiator.c $(LDLIBS)

example-pwcet.h: example example.ff
	../dumpcft ./example example-pwcet.h

example: example.c
	$(ARMCC) -g -O0 -o example example.c -static

example.ff: example
	@echo ""
	@echo "Please generate example.ff file using the mkff tool, and edit it to set the loop bounds"
	@echo ""
	@exit 1

.PHONY: clean tidy

tidy: 
	rm -f example-pwcet.h pwcet_instantiator *.o *~ *decomp* *.dot
	
clean: tidy
	rm -f example.ff example


