SOURCES=src/ocgeo.c src/sds.c src/cJSON.c
OBJ=$(SOURCES:.c=.o)
LIBNAME=libocgeo
LIB=$(LIBNAME).a

CURL_CONFIG = curl-config
CFLAGS += $(shell $(CURL_CONFIG) --cflags)
LIBS += $(shell $(CURL_CONFIG) --libs) -lm

all: $(LIB) example ocgeo_tests

ocgeo.o: ocgeo.c ocgeo.h

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

example: src/example.c $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

ocgeo_tests: tests/tests.c $(LIB)
	$(CC) $(CFLAGS) -Isrc $(LDFLAGS) -o $@ tests/tests.c $(LIB) $(LIBS)

test: ocgeo_tests
	@./$^

clean:
	rm -f example ocgeo_tests $(OBJ) *.a

.PHONY: clean all test
