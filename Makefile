NAME=cs
SRC=src/
HEADERS=headers/
TMP=tmp/

all:
	mkdir -p $(TMP)
	bison -d -t $(SRC)$(NAME).y
	mv $(NAME).tab.c $(TMP)
	mv $(NAME).tab.h $(TMP)

	flex -o $(TMP)$(NAME).yy.c $(SRC)$(NAME).lex
	gcc -c $(SRC)stable.c
	gcc -c $(SRC)quad.c
	gcc -c $(SRC)mips.c
	gcc -c $(SRC)list.c
	mv stable.o $(TMP)
	mv quad.o $(TMP)
	mv mips.o $(TMP)
	mv list.o $(TMP)
	gcc -o $(NAME).out $(TMP)stable.o $(TMP)quad.o $(TMP)list.o $(TMP)mips.o $(TMP)$(NAME).tab.c $(TMP)$(NAME).yy.c -lm -Werror -Wextra -g

clean:
	rm -rf $(TMP) $(NAME).out
