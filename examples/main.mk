SOURCES    = server.c client.c
OBJECTS    = $(SOURCES:.c=.o)
BINARIES   = server client

LDFLAGS += -lssl -lcrypto -ldl

default: all
all: $(BINARIES)

$(BINARIES): $(OBJECTS)
	$(CC) $(@:=.o) -o $@ $(LDFLAGS)

clean:
	rm -f $(BINARIES) $(OBJECTS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: default all clean
