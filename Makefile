build:
	gcc -Wall -Wextra -std=c99 -g tema3.c -lm -o image_editor
pack:
	zip -FSr 311CA_VladBalteanu_Tema3.zip tema3.c README Makefile
clean:
	rm -f image_editor