SOURCES=tests.c
OBJ=$(SOURCES:.c=.o)
LIBNAME=libocgeo
LIB=../src/$(LIBNAME).a

CFLAGS+=-Wall -Werror -fPIC -std=c99 -pedantic -O3 -I../src
LDFLAGS += -L../src
LDLIBS=-lcurl

all: tests

tests: tests.c $(LIB)

clean:
	rm -f tests $(OBJ)

.PHONY: clean all