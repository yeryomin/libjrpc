SOURCES    = server.c client.c
OBJECTS    = $(SOURCES:.c=.o)
BINARIES   = server client

default: all
all: $(BINARIES)

$(BINARIES): $(OBJECTS)
	$(CC) $(@:=.o) -o $@ $(LDFLAGS)

clean:
	rm -f $(BINARIES) $(OBJECTS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: default all clean
