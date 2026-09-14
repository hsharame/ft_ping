NAME        = ft_ping

CC          = gcc
CFLAGS      = -Wall -Wextra -Werror
RM          = rm -f

SRCS_DIR    = src
OBJS_DIR    = obj
INCS_DIR    = includes

SRCS_FILES  = main.c \
              socket.c \
              icmp.c \
              utils.c

SRCS        = $(addprefix $(SRCS_DIR)/, $(SRCS_FILES))
OBJS        = $(addprefix $(OBJS_DIR)/, $(SRCS_FILES:.c=.o))

INCLUDES    = -I $(INCS_DIR)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME) -lm

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c | $(OBJS_DIR)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

clean:
	@$(RM) -r $(OBJS_DIR)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
