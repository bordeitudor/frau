CC := clang
CFLAGS := -std=c11 -pedantic -Wall -Wextra -Wno-strict-prototypes -Wno-for-loop-analysis -I./

NAME := frau

ifeq ($(OS), Windows_NT)
	OUT := $(NAME).exe
else
	OUT := $(NAME)
endif

OBJ := obj/frau.o obj/stb_image.o
SRC := frau.c stb_image.c

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT)

obj/%.o: ./%.c
	$(CC) $(CFLAGS) -c $< -o $@
