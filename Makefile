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
	gcc -o $(NAME).out $(TMP)$(NAME).tab.c $(TMP)$(NAME).yy.c

clean:
	rm -rf $(TMP) $(NAME).out

