.POSIX:

CC            = cc
CFLAGS        = -Wall -Wextra -Wpedantic -g -I src
LDFLAGS       = 

TARGET        = cedit
PREFIX        = /usr/local
BINDIR        = $(PREFIX)/bin
MANDIR        = $(PREFIX)/share/man
SRCDIR        = src
BUILDDIR      = build

OBJFILES      = 

HEADERS       = 

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJFILES) $(BUILDDIR)/cedit.o
	$(CC) -o $@ $(OBJFILES) $(BUILDDIR)/cedit.o $(LDFLAGS)

$(BUILDDIR)/cedit.o: $(HEADERS) $(SRCDIR)/cedit.c
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/cedit.c

clean:
	rm -f $(BUILDDIR)/*

install: $(BUILDDIR)/$(TARGET)
	cp $(BUILDDIR)/$(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
