OCB = ocamlbuild
TARGET = swymplify
OCBFLAGS=-cflags -g

all: native
	mv main.native $(TARGET)

native:
	$(OCB) $(OCBFLAGS) main.native

byte:
	$(OCB) main.byte

debug:
	$(OCB) -yaccflag -v main.byte

clean:
	$(OCB) -clean
