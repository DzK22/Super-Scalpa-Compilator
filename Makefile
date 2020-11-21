NAME=cs
SRC=src/
HEADERS=headers/
TMP=tmp/

all:
	mkdir -p $(TMP)
	bison -d $(SRC)$(NAME).y
	mv $(NAME).tab.c $(TMP)
	mv $(NAME).tab.h $(TMP)

	flex -o $(TMP)$(NAME).yy.c $(SRC)$(NAME).lex
	gcc -c $(SRC)stable.c
	mv stable.o $(TMP)
	gcc -o $(NAME).out $(TMP)stable.o $(TMP)$(NAME).tab.c $(TMP)$(NAME).yy.c -lm -Werror -Wextra

clean:
	rm -rf $(TMP) $(NAME).out
