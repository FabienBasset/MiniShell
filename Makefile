all: minishell

minishell: tools.o Shell_vars.o Shell_commands.o main.o
	gcc -Iinclude -o minishell tools.o Shell_vars.o Shell_commands.o main.o

tools.o: src/tools.c
	gcc -c -Iinclude -o tools.o src/tools.c 

main.o: src/main.c include/Shell_vars.h include/Shell_commands.h include/tools.h
	gcc -c -Iinclude -o main.o  src/main.c 

Shell_vars.o: src/Shell_vars.c include/tools.h include/Shell_vars.h
	gcc -c -Iinclude -o Shell_vars.o src/Shell_vars.c

Shell_commands.o: src/Shell_commands.c include/tools.h include/Shell_vars.h
	gcc -c -Iinclude -o Shell_commands.o  src/Shell_commands.c


