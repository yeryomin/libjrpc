NAME    = libjrpc
SOURCES = $(NAME).c priv.c
HEADERS = $(NAME).h priv.h
OBJECTS = $(SOURCES:.c=.o)
SHARED  = $(NAME).so
STATIC  = $(NAME).a

default: all
all: $(SOURCES) $(SHARED) $(STATIC)

.c.o: $(HEADERS)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(SHARED): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -shared -o $@

$(STATIC): $(OBJECTS)
	$(AR) -rcv $@ $(OBJECTS)
	$(RANLIB) $(STATIC)

clean:
	rm -f *.o
	rm -f $(SHARED)
	rm -f $(STATIC)


.PHONY: default all $(SHARED) $(STATIC) clean
