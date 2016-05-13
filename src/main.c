/*
    Fichier main.c : Contient la boucle du shell et le traitement des commandes
    Auteurs : Fabien BASSET, Clément PALLUEL
    Dépendances : Shell_vars.h, Shell_commands.h, tools.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "Shell_vars.h"
#include "Shell_commands.h"
#include "tools.h"
/*

#include <sys/termios.h>
#include <sys/ioctl.h>

/*
    Fonction Terminal_Newline
    Calcul puis affiche une nouvelle ligne dans le shell
*/
void Terminal_Newline()
{
    char * user = getenv("USER");
    char * current = getenv("PWD");
    char * newline = NULL;

    if(strcmp(current,getenv("HOME"))==0)
    {
        newline = (char*)malloc((strlen(user)+30)*sizeof(char));
        sprintf(newline,"\033[32;1m%s\033[0;1m:\033[34;1m~ \033[0m$ ",user);
    }
    else
    {
        newline = (char*)malloc((strlen(user)+strlen(current)+29)*sizeof(char));
        sprintf(newline,"\033[32;1m%s\033[0;1m:\033[34;1m%s \033[0m$ ",user,current);
    }
    fputs(newline,stdout);
    fprintf(stdout,"\033[0m");
    free(newline);
}

/*
    Fonction Redirections_Parser
    Paramètre command : Adresse de la commande courante à analyser
    Paramètre symbol : symbol de redirection
    Détermine si il y a redirection
*/
char * Redirections_Parser(char ** command, const char * symbol)
{
    char ** args = Cut_String(*command,symbol);
    if(args[1])
    {
        char * new_command = malloc(sizeof(char)*(strlen(args[1])+1));
        strcpy(new_command,args[0]);

        char ** args2 = Cut_String(args[1]," ");
        char * file = malloc(sizeof(char)*(strlen(args2[0])+1));
        strcpy(file,args2[0]);
        int i=1;
        while(args2[i])
        {
            if(!(new_command = realloc(new_command,sizeof(char)*(strlen(args2[i])+strlen(new_command)+1))))
            {
                perror("Memory Allocation");
                exit(EXIT_FAILURE);
            }
            strcat(new_command,args[i++]);
        }
        Array_Free(args2,2);
        Array_Free(args,2);
        free(*command);
        *command = new_command;
        return(file);
    }
    else
    {
    	if(strlen(*command)>=strlen(symbol))
        	if(strcmp(*command+strlen(*command)-strlen(symbol),symbol)==0)
       	 		fprintf(stderr,"Syntaxe Error, expected file name after \"%s\".\n",symbol);
        Array_Free((void*)args,2);
        return(NULL);
    }
}

/*
    Fonction Command_Is_Shell_API
    Paramètre command_args : Pointeur sur tableau de chaines permettant de passer les arguments de la commande courante
    Retourne un entier indiquant l'id de la commande shell ou -1 si la commande n'est pas une commande shell
*/
int Command_Is_Shell_API(char ** command_args)
{
    int shell_api = 0;
    if(strcmp(command_args[0],"exit")==0) ///liste des commandes reconnues par le shell
        shell_api = 1;
    else if(strcmp(command_args[0],"cd")==0)
        shell_api = 2;
    else if(strcmp(command_args[0],"export")==0)
        shell_api = 3;
    else if(strcmp(command_args[0],"unset")==0)
        shell_api = 4;
    return(shell_api);
}

/*
    Fonction Command_Shell_Run
    Paramètre shell_args : Variable globale du shell contenant toutes les données comme les variables, les commandes etc.
    Paramètre args : arguments de la commande courante
    Paramètre command_id : id de la commande shell a lancer
    Exécute les commande shell.
*/
void Command_Shell_Run(SHELL_Context * context,char ** command_args,int command_id)
{
    switch(command_id) /// on éxécute les actions de chaque commande
    {
        case 1: exit_command(context);break;
        case 2: cd_command(command_args); break;
        case 3: export_command(command_args,context); break;
        case 4: unset_command(command_args,context); break;
    }
}

/*
    Fonction Command_Process
    Paramètre shell_args : Variable globale du shell contenant toutes les données comme les variables, les commandes etc.
    Lance le ou les fils exécutant la ou les commandes saisie(s)
*/
void Command_Process(SHELL_Context * context, char * input_file, char * output_file, int file_raise)
{
    int status = 0;
    /// CALCUL DE NB DE COMMANDES
    context->n_commands = 0;
    while(context->commands[context->n_commands])
        context->n_commands++;

    /// CREATION DES PIPES ///////////////////
    int i,j, index_pipe=0;
    int p[context->n_commands-1];
    for(i = 0; i < context->n_commands-1; i++){
        if(pipe(p + i*2) < 0) {
            perror("Error, Couldn't Pipe !");
            exit(EXIT_FAILURE);
        }
    }

    /// ON EXECUTE TOUTES LES COMMANDES //////////////////////////////
    pid_t pid;
    for (i = 0; i < context->n_commands; i++)
    {
        int command_id;
        char ** args = Cut_String(context->commands[i]," "); /// récupération des arguments de la commande courante
        Variables_CommandParser(context, context->vars,args);
        if((command_id = Command_Is_Shell_API(args))==0)
        {
            pid = fork();
            switch (pid)
            {
                case -1: fputs("Fork error\n",stderr);exit(-2);break;
                case 0:
                {
                        if(context->background==1)
                            printf("\n[%d] PID:[%d] \"%s\"\n",context->n_backgrounds,getpid(),context->commands[context->n_commands-1]);
                        signal(SIGINT, SIG_DFL);

                        /* redirection entrée */
                        if((i==0) && (input_file))
                        {
                                int fd = STDIN_FILENO;
                                if ((fd = open(input_file, O_RDONLY, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) < 0) {
                                    perror(input_file);
                                   // exit(EXIT_FAILURE);
                                   break;
                                }
                                dup2(fd, STDIN_FILENO);
                                close(fd);
                        }
                        /* **** */

                        if(i < context->n_commands-1) ///On rempli les tubes jusque la n-1 éme commande
                            if(dup2(p[index_pipe + 1], 1) < 0)
                            {
                                perror("Pipe Error");
                                exit(EXIT_FAILURE);
                            }

                        if(index_pipe != 0 ) ///On redirige la STDOUT vers l'entrée des pipes, sauf si on est a la premiere commande
                            if(dup2(p[index_pipe-2], 0) < 0)
                            {
                                perror("Pipe Error");
                                exit(EXIT_FAILURE);
                            }

                        for(j = 0; j < 2*i; j++) ///On ferme les pipes déjà utilisés par les commandes precedentes
                                close(p[j]);

                        /* redirection sortie */
                        if((i == context->n_commands-1) && (output_file))
                        {
                                int fd ;
                                if(file_raise)
                                {
                                    if ((fd = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) < 0) {
                                        perror(output_file);
                                        exit(EXIT_FAILURE);
                                    }
                                }
                                else
                                {
                                    if ((fd = open(output_file, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) < 0) {
                                        perror(output_file);
                                        exit(EXIT_FAILURE);
                                    }
                                }
                                dup2(fd, STDOUT_FILENO);
                                close(fd);
                        }
                        /* **** */

                        execvp(args[0], args);
                        perror(args[0]);
                        exit(EXIT_FAILURE);
                        break;
                }
                default:    index_pipe=index_pipe+2;   break;
            }
        }
        else
        {
            if(context->background==1)
                    printf("\n[%d] PID:[%d] \"%s\"\n",context->n_backgrounds,getpid(),context->commands[context->n_commands-1]);
            Command_Shell_Run(context,args,command_id);
            pid = getpid();
        }
        Array_Free(args,2);
    }

    /// On fermer les pipes ///////////////
    for(i = 0; i < 2 * (context->n_commands-1); i++)
        close(p[i]);

    /// on attend la fin de dernier processus et on gére le status et l'arriere plan ///
    if(context->background==1)
    {
        waitpid(pid,&status,0);
        fprintf(stdout,"\nProcess [%d] \"%s\" PID:[%d] terminated with status %d\n",context->n_backgrounds,context->commands[context->n_commands-1],pid,status);
    }
    int s = 0;
    while(wait(&s)!=-1)
        status+=s;
    context->last_return_value = status;
}

/*
    Fonction Commands_Parser
    Paramètre shell_args : Variable globale du shell contenant toutes les données comme les variables, les commandes etc.
    Paramètre meta_command : Pointeur sur chaine représentant la ligne saisie par l'utilisateur
    Découpe la meta commande et vérifie la syntaxe
*/
void Commands_Parser(SHELL_Context * context,char * meta_command)
{
    int i = 0, j, cursor, k, l;
    char ** commands = Cut_String(meta_command,";");
    for(i=0;commands[i];i++) /// COMMANDES SÉPARÉES PAR ";"
    {
        j = 0;
        char ** commands_orcond = Cut_String(commands[i],"||");
        do /// COMMANDES SEPARÉES PAR "||" (OU LOGIQUE)
        {
            k=0;
            char ** commands_andcond = Cut_String(commands_orcond[j],"&&");
            do /// COMMANDES SEPARÉES PAR "&&" (ET LOGIQUE)
            {
                l = cursor = 0;
                char ** commands_background = Cut_String(commands_andcond[k],"&");
                do /// COMMANDES EN ARRIERE PLAN
                {
                    if(strlen(commands_background[l]) != strlen(commands_andcond[k]+cursor)) // si la commande est en arriere plan 
                    {
                        if((commands_andcond[k+1]) || (commands_orcond[j+1])) // et s'il n'y a pas d'autre opérateur suivant la commande 
                        {
                            context->last_return_value = -1;
                            fprintf(stderr,"Syntaxe error, unexpected \"&\".\n");
                            break;
                        }
                        else
                            context->background = 1;
                    }
                    else
                        context->background = 0;
                    cursor = cursor + strlen(commands_background[l]) + 1; // curseur de parcour de la commande 

                    /* CHECK REDIRECTIONS */
                    int file_raise = 1;
                    char * output_file, *input_file;
                    if((output_file = Redirections_Parser(&commands_background[l],">>")))
                        file_raise = 0;
                    else
                        output_file = Redirections_Parser(&commands_background[l],">");
                    input_file = Redirections_Parser(&commands_background[l],"<");
                    /* ****************** */

                    context->commands = Cut_String(commands_background[l],"|"); /// COMMANDES TUBÉES

                    /* On lance les commandes */
                    if(context->background==0)
                        Command_Process(context,input_file,output_file,file_raise);
                    else
                    {
                        pid_t b_pid = fork();
                        context->n_backgrounds++;
                        if(b_pid==0)
                        {
                            Command_Process(context,input_file,output_file,file_raise);
                            exit(0);
                        }
                    }
                    /* ********************** */
					free(input_file);
					free(output_file);
                    Array_Free(context->commands,2);
                }
                while((commands_background[++l]) && (context->last_return_value==0));
                Array_Free(commands_background,2);
            }
            while((commands_andcond[++k])  && (context->last_return_value==0));
            Array_Free(commands_andcond,2);
        }
        while((commands_orcond[++j]) && (context->last_return_value!=0));
        Array_Free(commands_orcond,2);
    }
    Array_Free(commands,2);
}


int main()
{
    //signal(SIGINT, SIG_IGN); /// On désactive le signal de CTRL-C pour le shell
    printf("\n      HELLO, %s !\n\n",getenv("USER"));

    ///On initialise la liste des variables essentielles pour le shell
    SHELL_Context * context = (SHELL_Context*)malloc(sizeof(SHELL_Context));
    context->vars = Variables_Create();
    context->commands = NULL;
    context->n_commands = 0;
    context->last_return_value = 0;
    context->background = 0;
    context->n_backgrounds = 0;

    while(1) /// boucle infinie du shell
    {
        Terminal_Newline(); /// On ajoute une ligne
        char * command = Read_Input(512); /// On récupére la commande saisie
        Commands_Parser(context,command);
		free(command);
    }
    
	/*struct termios term_before, term_after;
	char c;
	ioctl(0, TCGETS, &term_before);
	
	ioctl(0, TCGETS, &term_after);
	term_after.c_lflag &=  ~ECHO;
	term_after.c_lflag &=  ~ICANON;
	ioctl(0, TCSETS, &term_after);
	
	char * test = NULL;
	int current=0, size=0, i;
	while ((c = getc(stdin))!='\n')
	{
	  	if(c==27)
	  	{
	  		if((c = getc(stdin))==91)
	  		{
	  			if((c = getc(stdin))==68)
	  			{
	  				if(current>0) 
	  					current--;	 			
					printf("\b");
	  			}
	  			//else if((c = getc(stdin))==67)
	  				//printf("\r");
			}
		}
	  	else if(c==127)
	  	{
	  		if(current>0)
	  		{
		  		current--;
		  		for(i=current;i<size-1;i++)
		  			test[i] = test[i+1];
		  		
	  			test = realloc(test,sizeof(char)*(size));
	  			test[size--] = '\0';

		  		printf("\b \b%s\b \b",test+current);
		  		for(i=0;i<size-current;i++)
	  				printf("\b");
	  		}

	  	}
	  	else
	  	{
	  		test = realloc(test,sizeof(char)*(size+2));
	  		for(i=size;i>current;i--)
	  			test[i] = test[i-1];
	  		
	  		test[current++] = c;
	  		test[++size] = '\0';
	  			
  			printf("%s",test+current-1);
  			for(i=0;i<size-current;i++)
  				printf("\b");
	  	}
	}  	
	test = realloc(test,sizeof(char)*(size+1));
	test[size] = '\0';
	printf("\n%s\n",test);
	
	  	
	ioctl(0, TCSETS, &term_before);*/
	
	return(0);
}
