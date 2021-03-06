NAME=cs
SRC=src/
HEADERS=headers/
TMP=tmp/
MIPS=tests/mips/
COMPIL=tests/compil/
CC= gcc -g
CFLAGS= -Wall -Werror -Wextra

all:
	bison --warnings=all -d -t $(SRC)$(NAME).y -v
	mkdir -p $(TMP)
	mv $(NAME).tab.c $(TMP)
	mv $(NAME).tab.h $(TMP)

	flex -o $(TMP)$(NAME).yy.c $(SRC)$(NAME).lex
	$(CC) -c $(SRC)stable.c $(CFLAGS)
	$(CC) -c $(SRC)quad.c $(CFLAGS)
	$(CC) -c $(SRC)mips.c $(CFLAGS)
	$(CC) -c $(SRC)list.c $(CFLAGS)
	$(CC) -c $(SRC)opti.c $(CFLAGS)
	$(CC) -c $(SRC)array.c $(CFLAGS)
	$(CC) -c $(SRC)util.c $(CFLAGS)

	mv *.o $(TMP)

	$(CC) -o scalpa $(TMP)stable.o $(TMP)quad.o $(TMP)list.o $(TMP)array.o $(TMP)mips.o $(TMP)opti.o $(TMP)util.o $(TMP)$(NAME).tab.c $(TMP)$(NAME).yy.c -lm -Werror -Wextra -g

test:
	./tests/run.sh

clean:
	rm -rf $(TMP) scalpa $(COMPIL) $(MIPS) *.s tmp_res *.output
