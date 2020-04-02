ARMCC=arm-none-eabi-gcc
ARMCFLAGS=-nostdlib -nostdinc

CFLAGS=-O0 -g -Wall -W -fsanitize=address
LDLIBS=../pwcet/lib/libpwcet-runtime.a
LDFLAGS=-g -fsanitize=address

all: pwcet_instantiator example.pwf

pwcet_instantiator: pwcet_instantiator.c example-pwcet.h ../pwcet/include/pwcet-runtime.h ../pwcet/lib/libpwcet-runtime.a
	cc $(CFLAGS) -c pwcet_instantiator.c
	cc $(LDFLAGS) -o pwcet_instantiator pwcet_instantiator.c $(LDLIBS)

example-pwcet.h example.pwf: example example.ff
	../dumpcft ./example example-pwcet.h example.pwf

example: example.c
	$(ARMCC) $(ARMCFLAGS) -g -O0 -o example example.c -static

example.ff: example
	@echo ""
	@echo "Please generate example.ff file using 'mkff example>example.ff', then edit it to set the loop bounds"
	@echo ""
	@exit 1

.PHONY: clean tidy

tidy: 
	rm -f example-pwcet.h pwcet_instantiator *.o *~ *decomp* *.dot

clean: tidy
	rm -f example.ff example


