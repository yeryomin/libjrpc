CC       = $(CROSS_COMPILE)gcc
LD       = $(CROSS_COMPILE)ld

CFLAGS  += -Wall -fPIC -O3 -g3
LDFLAGS += -static

SRCDIR   = ../..

libs     = jrpc ipsc fmt a
libsrc   = $(addprefix $(SRCDIR)/lib, $(libs))
CFLAGS  += $(addprefix -I, $(libsrc))
LDFLAGS += $(addprefix -L, $(libsrc)) $(addprefix -l, $(libs))

jsmnsrc  = $(SRCDIR)/jsmn
CFLAGS  += -I$(jsmnsrc)

LDFLAGS += -lrt -lpthread


include main.mk
