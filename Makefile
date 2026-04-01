.POSIX:

CC            = cc
CFLAGS        = -std=c89 -Wall -Wextra -Wpedantic -g -I src
LDFLAGS       = -lcurses

TARGET        = cedit
PREFIX        = /usr/local
BINDIR        = $(PREFIX)/bin
MANDIR        = $(PREFIX)/share/man
SRCDIR        = src
BUILDDIR      = build

OBJFILES      = $(BUILDDIR)/cedit.o\
                $(BUILDDIR)/string.o $(BUILDDIR)/list_string.o

HEADERS       = $(SRCDIR)/jd297/string.h $(SRCDIR)/jd297/list_string.h

$(BUILDDIR)/$(TARGET): $(OBJFILES)
	$(CC) -o $@ $(OBJFILES) $(LDFLAGS)

$(BUILDDIR)/cedit.o: $(HEADERS) $(SRCDIR)/cedit.c
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/cedit.c

$(BUILDDIR)/string.o: $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ -DJD297_STRING_IMPLEMENTATION -x c $(SRCDIR)/jd297/string.h

$(BUILDDIR)/list_string.o: $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ -DJD297_LIST_IMPLEMENTATION -x c $(SRCDIR)/jd297/list_string.h

clean:
	rm -f $(BUILDDIR)/*

install: $(BUILDDIR)/$(TARGET)
	cp $(BUILDDIR)/$(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
