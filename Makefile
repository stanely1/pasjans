CC=gcc
CFLAGS= -std=c11 -xc -Wno-deprecated-declarations
LFLAGS= `pkg-config gtk+-3.0 --cflags --libs`

SRC=src/main.c src/stack.c
NAME=pasjans

$(NAME): $(SRC)
	$(CC) $(CFLAGS) -o $(NAME) $(SRC) $(LFLAGS)

clean:
	rm $(NAME)