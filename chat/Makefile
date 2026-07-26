CC     = cc
CFLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined
# Session 2 で linebuf.c を足す
SRCS   = main.c net.c

chat: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@

clean:
	rm -f chat
	rm -rf chat.dSYM

.PHONY: clean
