SOURCES=src/ocgeo.c src/sds.c src/cJSON.c
OBJ=$(SOURCES:.c=.o)
LIBNAME=libocgeo
LIB=$(LIBNAME).a

CFLAGS+=-Wall -Werror -fPIC -std=c99 -pedantic -O2
LDLIBS=-lcurl -lm

all: $(LIB) example ocgeo_tests

ocgeo.o: ocgeo.c ocgeo.h

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

example: src/example.c $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ocgeo_tests: tests/tests.c $(LIB)
	$(CC) $(CFLAGS) -Isrc $(LDFLAGS) -o $@ tests/tests.c $(LIB) $(LDLIBS)

test: ocgeo_tests
	@./$^

clean:
	rm -f example ocgeo_tests $(OBJ) *.a

.PHONY: clean all test
