SOURCES=ocgeo.c sds.c cJSON.c
OBJ=$(SOURCES:.c=.o)
LIB=libocgeo
STATIC=$(LIB).a

CFLAGS+=-Wall -Werror -fPIC -std=c99 -pedantic -O3
LDLIBS=-lcurl

all: $(STATIC) example

$(STATIC): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

example: example.c $(STATIC)

clean:
	rm -f example $(OBJ) *.a

.PHONY: clean all