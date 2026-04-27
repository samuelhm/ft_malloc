CC = cc

CFLAGS = -Wall -Wextra -Werror -fPIC
CFLAGS += -Wshadow -Wcast-qual -Wcast-align
CFLAGS += -Wwrite-strings -Wstrict-overflow=2
CFLAGS += -Wformat=2 -Wbad-function-cast -Wnested-externs

DEBUG_FLAGS = -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer

INCLUDES = -Iinclude -Ilibft/inc

NAME = libft_malloc_$(HOSTTYPE).so
LIBFT = libft/libft.a
LIBFT_DIR = libft
LIBFT_REPO = https://github.com/samuelhm/Libft42.git
SRC = src/malloc.c \
	  src/free.c \
	  src/realloc.c \
	  src/zone.c \
	  src/block.c \
	  src/show_alloc_mem.c \
	  src/utils/debug.c \
	  bonus/thread_safe.c \
	  bonus/show_alloc_mem_ex.c \
	  bonus/defrag.c

ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(LIBFT):
	@if [ ! -f "$(LIBFT_DIR)/Makefile" ]; then \
		echo "Fetching $(LIBFT_DIR)..."; \
		git submodule update --init --recursive $(LIBFT_DIR) || { rm -rf $(LIBFT_DIR) && git clone $(LIBFT_REPO) $(LIBFT_DIR); }; \
	fi
	$(MAKE) -C $(LIBFT_DIR)

$(NAME): $(OBJ) $(LIBFT)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJ) -Llibft -lft
	ln -sf $(NAME) libft_malloc.so

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(OBJ) $(LIBFT)
	$(CC) $(CFLAGS) -shared -o $(NAME) $(OBJ) -Llibft -lft
	ln -sf $(NAME) libft_malloc.so

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJ)
	@if [ -d "$(LIBFT_DIR)" ]; then $(MAKE) -C $(LIBFT_DIR) clean; fi

fclean: clean
	@if [ -d "$(LIBFT_DIR)" ]; then $(MAKE) -C $(LIBFT_DIR) fclean; fi
	rm -f $(NAME) libft_malloc.so

re: fclean all

.PHONY: all clean fclean re debug
