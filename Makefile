GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
CFLAGS=-Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -DGIT_VERSION=\"$(GIT_VERSION)\"
CC = gcc

OBJS = otar \

all: otar

otar: otar.c
	$(CC) $(CFLAGS) otar.c -o otar

clean:
	rm -f $(OBJS)
