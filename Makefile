CC=gcc
CFLAGS= -std=c11 -xc -Wno-deprecated-declarations
LFLAGS= `pkg-config gtk+-3.0 --cflags --libs`

SRCDIR=src

SRC=$(wildcard $(SRCDIR)/*.c)
DEPS=$(wildcard $(SRCDIR)/*.h)

NAME=pasjans

$(NAME): $(SRC) $(DEPS)
	$(CC) $(CFLAGS) -o $(NAME) $(SRC) $(LFLAGS)

clean:
	rm $(NAME)
