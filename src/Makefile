SOURCES=ocgeo.c sds.c cJSON.c
OBJ=$(SOURCES:.c=.o)
LIB=libocgeo
STATIC=$(LIB).a

GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"

CFLAGS+=-Wall -Werror -fPIC -std=c99 -pedantic -O3 -DOCGEO_VERSION=\"$(GIT_VERSION)\"
LDLIBS=-lcurl

all: $(STATIC) example

$(STATIC): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

example: example.c $(STATIC)

clean:
	rm -f example $(OBJ) *.a

.PHONY: clean all