/*
    Fichier Shell_commands.c : Contient les commandes du shell
    Auteurs : Fabien BASSET, Clément PALLUEL
    Dépendances : Shell_vars.h, tools.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tools.h"
#include "Shell_vars.h"

/*
    Fonction cd_command
    Paramètre args : Pointeur sur tableau de chaines représentant les arguments de la commande courante
    Change de répertoire courant
*/
void cd_command(char ** args)
{
    if(args[1])
    {
        if(args[1][0]=='/')
        {
            if(chdir(args[1])!=0)
                fprintf(stderr,"cd : %s : No such file or directory\n",args[1]);
            else
                setenv("PWD",args[1],1);
        }
        else
        {
            char * current = getenv("PWD");
            char * new_current = (char*)malloc(sizeof(char)*(strlen(current)+strlen(args[1])+1));
            if(strcmp(current,"/")==0) /// test si on est a la racine
                sprintf(new_current,"%s%s",current,args[1]);
            else
                sprintf(new_current,"%s/%s",current,args[1]);
            if(chdir(new_current)!=0)
            {
                fprintf(stderr,"cd : %s : No such file or directory\n",new_current);
                return;
            }
            else
                setenv("PWD",new_current,1);
            free(new_current);
        }
        /// on supprime les ".." ////////////////////////////////////////////////////////////
        char ** dir_cut = Cut_String(getenv("PWD"),"/");
        int i=0, j;
        while(dir_cut[i]) i++;
        int cd_dirs[i];
        for(j=0;j<i;j++)
        {
            if(strcmp(dir_cut[j],"..")==0)
            {
                cd_dirs[j] = 0;
                int k = 1;
                while((cd_dirs[j-k]==0) && (j-k>=0))
                    k++;
                if(j-k>=0)
                    cd_dirs[j-k] = 0;
            }
            else
                cd_dirs[j] = 1;
        }
        char * cleared_current = NULL;int size =0;
        for(j=0;j<i;j++)
        {
            if(cd_dirs[j]==1)
            {
                if(cleared_current)
                    size = strlen(cleared_current);
                if(!(cleared_current = realloc(cleared_current,sizeof(char)*(strlen(dir_cut[j])+1+size))))
                {
                    fprintf(stderr,"Error, too weak memory");
                    exit(EXIT_FAILURE);
                }
                if(cleared_current)
                    sprintf(cleared_current,"%s/%s",cleared_current,dir_cut[j]);
                else
                    sprintf(cleared_current,"/%s",dir_cut[j]);
            }
        }
        if(!cleared_current)
            setenv("PWD","/",1);
        else
            setenv("PWD",cleared_current,1);
        free(cleared_current);
        /// /////////////////////////////////////////////////////////////////////////////////
    }
    else
    {
        chdir(getenv("HOME"));
        setenv("PWD",getenv("HOME"),1);
    }
}
/*
    Fonction export_command
    Paramètre args : Pointeur sur tableau de chaines représentant les arguments de la commande courante
    Paramètre shell_args : Pointeur sur Objet Shell_Args représentant la variable globale du shell
    Créé une variable
*/
void export_command(char ** args,SHELL_Context * context)
{
    if((!args[1])) /* cas export sans argument => on affiche toutes les variables */
    {
        int i,type;
        fprintf(stdout,"\n------------ [ VARS ] ------------\n");
        fprintf(stdout," ** SHELLVAR \"?\" = \"%d\" , type : REAL\n",context->last_return_value);
        fprintf(stdout," ** SHELLVAR \"§\" = \"%d\" , type : REAL\n\n",context->n_backgrounds);
        for(i=0;i<context->vars->n_var;i++)
        {
            type =  Variables_GetType(context->vars,context->vars->names[i]);
            if(type==REAL)
                fprintf(stdout," => VAR \"%s\" = \"%s\" , type : REAL\n",context->vars->names[i],context->vars->values[i]);
            else if(type==BOOLEAN)
               fprintf(stdout," => VAR \"%s\" = \"%s\" , type : BOOLEAN\n",context->vars->names[i],context->vars->values[i]);
            else if(type==STRING)
                fprintf(stdout," => VAR \"%s\" = \"%s\" , type : ALPHANUMERIC\n",context->vars->names[i],context->vars->values[i]);
        }
        if(i==0)
            fprintf(stdout,"No declared vars.\n");
        else
            fprintf(stdout,"%d declared var(s).\n",context->vars->n_var);
        fprintf(stdout,"----------------------------------\n\n");
    }
    else /* sinon on déclare une variable */
    {
        char ** var_cut = Cut_String(args[1],"=");
        if(!var_cut[1])
            fputs("export : syntaxe error\n",stderr);
        else
        {
            if( (strchr(var_cut[0],'(')) || (strchr(var_cut[0],')')) || (strchr(var_cut[0],'$')) || (strchr(var_cut[0],'/')) || (strchr(var_cut[0],'\\')) || (strchr(var_cut[0],'+')) || (strchr(var_cut[0],'-')) || (strchr(var_cut[0],'*')) /// TEST SUR LE NOM DE VARIABLE
            || (var_cut[0][0]=='0') || (var_cut[0][0]=='1') || (var_cut[0][0]=='2') || (var_cut[0][0]=='3') || (var_cut[0][0]=='4') || (var_cut[0][0]=='5')
            || (var_cut[0][0]=='6') || (var_cut[0][0]=='7') || (var_cut[0][0]=='8') || (var_cut[0][0]=='9') || (strchr(var_cut[0],'?')) || ((strchr(var_cut[0],'§'))) )
                fprintf(stderr,"Error : illegal name \"%s\".\n",var_cut[0]);
            else if(!Variables_GetValue(context->vars,var_cut[0]))
            {
                Variables_Add(context->vars,var_cut[0],var_cut[1]);
                fprintf(stdout,"Var %s succesfully declared.\n",var_cut[0]);
            }
            else
            {
                Variables_SetValue(context->vars,var_cut[0],var_cut[1]);
                fprintf(stdout,"Var %s changed.\n",var_cut[0]);
            }
        }
        Array_Free(var_cut,2);
    }
}
/*
    Fonction unset_command
    Paramètre args : Pointeur sur tableau de chaines représentant les arguments de la commande courante
    Paramètre shell_args : Pointeur sur Objet Shell_Args représentant la variable globale du shell
    Supprime une variable
*/
void unset_command(char ** args,SHELL_Context * context)
{
    if(args[1])
    {
        if(Variables_Remove(&context->vars,args[1])==-1)
            fprintf(stderr,"Error : unknown var \"%s\".\n",args[1]);
        else
            fprintf(stdout,"Var \"%s\" deleted.\n",args[1]);
    }
    else
        fputs("unset : syntaxe error\n",stderr);
}
/*
    Fonction exit_command
    Paramètre shell_args : Pointeur sur Objet Shell_Args représentant la variable globale du shell
    Libère la mémoire de la variable gobale du shell puis quitte le shell
*/
void exit_command(SHELL_Context * context)
{
    fprintf(stdout,"\n      GOOD BYE %s !     \n",getenv("USER"));
    Free_Variables(context->vars);
    Array_Free(context->commands,2);
    free(context);
    exit(0);
}
