NAME=cs
SRC=src/
HEADERS=headers/
TMP=tmp/
MIPS=mips/


all:
	mkdir -p $(TMP)
	bison -d -t $(SRC)$(NAME).y
	mv $(NAME).tab.c $(TMP)
	mv $(NAME).tab.h $(TMP)

	flex -o $(TMP)$(NAME).yy.c $(SRC)$(NAME).lex
	gcc -c $(SRC)stable.c
	gcc -c $(SRC)quad.c
	gcc -c $(SRC)mips.c
	gcc -c $(SRC)arglist.c
	gcc -c $(SRC)opti.c

	mv stable.o $(TMP)
	mv quad.o $(TMP)
	mv mips.o $(TMP)
	mv arglist.o $(TMP)
	mv opti.o $(TMP)

	gcc -o scalpa $(TMP)stable.o $(TMP)quad.o $(TMP)arglist.o $(TMP)mips.o $(TMP)opti.o $(TMP)$(NAME).tab.c $(TMP)$(NAME).yy.c -lm -Werror -Wextra -g

clean:
	rm -rf $(TMP) scalpa $(MIPS) *.s
