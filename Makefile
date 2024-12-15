my_shell : my_shell.c command.h command.c
	gcc -o my_shell -Wall my_shell.c command.c