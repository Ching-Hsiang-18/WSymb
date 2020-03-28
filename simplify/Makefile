OCB = ocamlbuild
TARGET = simplify

all: native
	mv main.native $(TARGET)

native:
	$(OCB) main.native

byte:
	$(OCB) main.byte

debug:
	$(OCB) -yaccflag -v

clean:
	$(OCB) -clean
