CFLAGS=-O0 -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow
LIBS=-lm
EXECUTAVEL=arvoreB

all: $(EXECUTAVEL)

$(EXECUTAVEL): pagina.o main.o
	gcc pagina.o main.o  $(LIBS) -o $(EXECUTAVEL)


pagina.o: pagina.c
	gcc $(CFLAGS) -c pagina.c

main.o: main.c
	gcc $(CFLAGS) -c main.c

clean:
	rm -rf *.o $(EXECUTAVEL)

memcheck: $(EXECUTAVEL)
	valgrind --tool=memcheck --leak-check=full ./$(EXECUTAVEL)

run: $(EXECUTAVEL)
	./$(EXECUTAVEL)