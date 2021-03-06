CC = gcc
CFLAGS = -Og -Wall -Wextra -pedantic -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
LIBS = -lpthread -lncurses -lm
DEFINES = -D_GNU_SOURCE -D_XOPEN_SOURCE=500
OBJS = util_funs.o menu_funs.o game_funs.o print_funs.o
ARCHIVE_NAME = rogulskip
DESTINATION = rogulskip@ssh.mini.pw.edu.pl:/home2/samba/sobotkap/unix

main: main.c $(OBJS)
	$(CC) -o $@ $< $(OBJS) $(CFLAGS) $(DEFINES) $(LIBS)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS) $(DEFINES) $(LIBS)

.PHONY: clean
clean:
	-rm -f main $(OBJS)

tar: main
	-tar czf $(ARCHIVE_NAME).tar.gz Makefile *.c *.h

push: tar
	-scp $(ARCHIVE_NAME).tar.gz $(DESTINATION)
