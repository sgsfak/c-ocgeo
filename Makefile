SOURCES=src/ocgeo.c src/sds.c src/cJSON.c
OBJ=$(SOURCES:.c=.o)
LIBNAME=libocgeo
LIB=$(LIBNAME).a

GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"

CFLAGS+=-Wall -Werror -fPIC -std=c99 -pedantic -O3 -DOCGEO_VERSION=\"$(GIT_VERSION)\"
LDLIBS=-lcurl

all: $(LIB) example ocgeo_tests

ocgeo.o: ocgeo.c ocgeo.h

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

example: src/example.c $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ocgeo_tests: tests/tests.c $(LIB)
	$(CC) $(CFLAGS) -Isrc $(LDFLAGS) -o $@ $^ $(LDLIBS)

test: ocgeo_tests
	@./$<

clean:
	rm -f example ocgeo_tests $(OBJ) *.a

.PHONY: clean all test
