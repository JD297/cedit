.POSIX:

CC            = cc
CFLAGS        = -std=c89 -Wall -Wextra -Wpedantic -g -I src -I .
LDFLAGS       = -lcurses

TARGET        = cedit
PREFIX        = /usr/local
BINDIR        = $(PREFIX)/bin
MANDIR        = $(PREFIX)/share/man
SRCDIR        = src
BUILDDIR      = build

BUILDINFO     = $(BUILDDIR)/BUILDINFO.h

OBJFILES      = $(BUILDDIR)/cedit.o\
                $(BUILDDIR)/string.o $(BUILDDIR)/list_string.o

HEADERS       = $(BUILDINFO)\
                $(SRCDIR)/jd297/string.h $(SRCDIR)/jd297/list_string.h

$(BUILDDIR)/$(TARGET): $(OBJFILES)
	$(CC) -o $@ $(OBJFILES) $(LDFLAGS)

$(BUILDDIR)/cedit.o: $(HEADERS) $(SRCDIR)/cedit.c
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/cedit.c

$(BUILDDIR)/string.o: $(HEADERS)
	$(CC) $(CFLAGS) -o $@.testsuite -DJD297_STRING_TESTSUITE -x c $(SRCDIR)/jd297/string.h && $@.testsuite
	$(CC) $(CFLAGS) -c -o $@ -DJD297_STRING_IMPLEMENTATION -x c $(SRCDIR)/jd297/string.h

$(BUILDDIR)/list_string.o: $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ -DJD297_LIST_IMPLEMENTATION -x c $(SRCDIR)/jd297/list_string.h

$(BUILDDIR)/BUILDINFO.h: src/BUILDINFO.h.template Makefile .git/logs/HEAD
	@cp src/BUILDINFO.h.template $(BUILDINFO)
	@printf "#define __COMMIT__ \"%s\"\n" "$$(git rev-parse --short HEAD)" >> $(BUILDINFO)
	@printf "#define __UNAME__ \"%s\"\n" "$$(uname -mrsv)" >> $(BUILDINFO)
	@printf "#define __CC__ \"%s\"\n" "$(CC)" >> $(BUILDINFO)
	@printf "#define __CC_VERSION__ \"%s\"\n" "$$($(CC) -v 2>&1 | head -n 1)" >> $(BUILDINFO)
	@printf "#define __CFLAGS__ \"%s\"\n" "$(CFLAGS)" >> $(BUILDINFO)
	@printf "#define __LDFLAGS__ \"%s\"\n" "$(LDFLAGS)" >> $(BUILDINFO)

clean:
	rm -f $(BUILDDIR)/*

install: $(BUILDDIR)/$(TARGET)
	cp $(BUILDDIR)/$(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)
