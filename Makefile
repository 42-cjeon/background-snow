NAME = snow
LDFLAGS = -fsanitize=address -lncurses
CFLAGS = -g 

SRCS=main.c
OBJS=$(SRCS:.c=.o)

$(NAME) : $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

clean :
	rm -v $(OBJS) $(NAME)