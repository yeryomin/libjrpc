CC       = $(CROSS_COMPILE)gcc
LD       = $(CROSS_COMPILE)ld

CFLAGS  += -Wall -fPIC -O3 -g3
#LDFLAGS += -static

SRCDIR   = ../..

libs     = jrpc ipsc a json-c
libsrc   = $(addprefix $(SRCDIR)/lib, $(libs))
CFLAGS  += $(addprefix -I, $(libsrc))
LDFLAGS += $(addprefix -L, $(libsrc)) $(addprefix -l, $(libs))

LDFLAGS += -lrt -lpthread

include main.mk
