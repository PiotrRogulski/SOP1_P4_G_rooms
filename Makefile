CC = gcc
CFLAGS = -Wall -Wextra -pedantic -fanalyzer
LIBS = -lpthread -lncurses
OBJS = util_funs.o game_funs.o

main: main.c $(OBJS)
	$(CC) -o $@ $< $(OBJS) $(CFLAGS) $(LIBS)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	-rm -f main $(OBJS)

tar%: all
	-tar czf $(ARCHIVE_NAME)$(*).tar.gz Makefile *.c

push%: tar%
	-scp $(ARCHIVE_NAME)$(*).tar.gz rogulskip@ssh.mini.pw.edu.pl:/home2/samba/sobotkap/unix
