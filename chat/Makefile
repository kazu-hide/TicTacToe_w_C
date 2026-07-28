CC     = cc
CFLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined
SRCS   = main.c net.c linebuf.c

chat: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@

clean:
	rm -f chat
	rm -rf chat.dSYM

.PHONY: clean
