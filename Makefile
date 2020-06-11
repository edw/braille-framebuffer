SRCS = $(shell ls *.c)
OBJS = $(SRCS:%.c=%.o)
DEPENDS = Makefile.deps

# CC = clang
CFLAGS = -O0 -g -std=c89 -D _GNU_SOURCE
LDFLAGS = -lm

demo: $(OBJS)

clean:
	rm -f *.o $(DEPENDS)

depend:
	$(CC) -MM -MF - $(CFLAGS) $(SRCS) > $(DEPENDS)

-include $(DEPENDS)
