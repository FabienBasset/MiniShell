/*
    Fichier Shell_vars.c : Contient les fonctions de gestion des variables du shell et la variable globale passé au shell durant tout le programme
    Auteurs : Fabien BASSET, Clément PALLUEL
    Dépendances : Shell_vars.h, tools.h
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"
#include "Shell_vars.h"


SHELL_Variables * Variables_Create()
{
    SHELL_Variables * vars = (SHELL_Variables*)malloc(sizeof(SHELL_Variables));
    vars->names = NULL;
    vars->values = NULL;
    vars->n_var = 0;
    return(vars);
}

/*
    Fonction Free_Variables
    Paramètre vars : Pointeur sur objet Variable représentant les variables du shell et leurs valeurs
    Libère la mémoire allouée
*/
void Free_Variables(SHELL_Variables * vars)
{
    int i;
    for(i=0;i<vars->n_var;i++)
    {
        free(vars->names[i]);
        free(vars->values[i]);
    }
    free(vars->names);
    free(vars->values);
    free(vars);
}

/*
    Fonction Variables_Add
    Paramètre vars : Pointeur sur objet Variable représentant les variables du shell et leurs valeurs
    Paramètre name : Pointeur sur chaine représentant le nom de la variable
    Paramètre value : Pointeur sur chaine représentant la valeur de la variable
    Ajoute une variable
*/
void Variables_Add(SHELL_Variables * vars,char * name,char * value)
{
    if(!(vars->names = realloc(vars->names,++vars->n_var*sizeof(char*))))
    {
        fputs("Too weak memory\n",stderr);
        exit(-1);
    }
    if(!(vars->values = realloc(vars->values,vars->n_var*sizeof(char*))))
    {
        fputs("Too weak memory\n",stderr);
        exit(-1);
    }

    vars->names[vars->n_var-1] = (char*)malloc(sizeof(char)*(strlen(name)+1));
    vars->values[vars->n_var-1] = (char*)malloc(sizeof(char)*(strlen(value)+1));
    sprintf(vars->names[vars->n_var-1],"%s",name);
    sprintf(vars->values[vars->n_var-1],"%s",value);
}

/*
    Fonction Variables_Remove
    Paramètre vars : Pointeur sur objet Variable représentant les variables du shell et leurs valeurs
    Paramètre name : Pointeur sur chaine représentant le nom de la variable
    Supprime une variable et retourne un entier représentant le succés de l'opération
*/
int Variables_Remove(SHELL_Variables ** vars,char * name)
{
    int i=0,j;
    while ((i<(*vars)->n_var) && (strcmp((*vars)->names[i],name)!=0)) i++; //on cherche la variable a supprimer
    if(i==(*vars)->n_var) //si elle n'existe pas, erreur !
    {
        return(-1);
    }
    //vars->names[i] = NULL;
    //vars->values[i] = NULL;
    SHELL_Variables * new_vars = Variables_Create(); // on recréé un tableau de variables
    for (j=0;j<i;j++)
        Variables_Add(new_vars,(*vars)->names[j],(*vars)->values[j]);
    for (j=i+1;j<(*vars)->n_var;j++)
        Variables_Add(new_vars,(*vars)->names[j],(*vars)->values[j]);
    Free_Variables(*vars);
    *vars = new_vars; //puis on remplace l'ancien par le nouveau
    return(0);
}
/*
    Fonction Variables_GetValue
    Paramètre vars : Pointeur sur objet Variable représentant les variables du shell et leurs valeurs
    Paramètre name : Pointeur sur chaine représentant le nom de la variable
    Retourne la valeur d'une variable
*/
char * Variables_GetValue(SHELL_Variables * vars,char * name)
{
    int i=0;
    while ((i<vars->n_var) && (strcmp(vars->names[i],name)!=0)) i++;
    if(i==vars->n_var) //si elle n'existe pas, erreur !
        return(NULL);
    else
        return(vars->values[i]);
}
/*
    Fonction Variables_SetValue
    Paramètre vars : Pointeur sur objet Variable représentant les variables du shell et leurs valeurs
    Paramètre name : Pointeur sur chaine représentant le nom de la variable
    Paramètre value : Pointeur sur chaine représentant la valeur de la variable
    Définie la valeur d'une variable puis retourne un entier représentant le succés de l'opération
*/
int Variables_SetValue(SHELL_Variables * vars,char * name,char * value)
{
    int i=0;
    while ((i<vars->n_var) && (strcmp(vars->names[i],name)!=0)) i++;
    if(i==vars->n_var) //si elle n'existe pas, erreur !
        return(-1);
    else
    {
        free(vars->values[i]);
        vars->values[i] = (char*)malloc(strlen(value)*sizeof(char));
        strcpy(vars->values[i],value);
        return(0);
    }
}
/*
    Fonction Variables_GetType
    Paramètre vars : Pointeur sur objet Variable représentant les variables du shell et leurs valeurs
    Paramètre name : Pointeur sur chaine représentant le nom de la variable
    Calcul puis retourne le type d'une variable
*/
int Variables_GetType(SHELL_Variables * vars,char * name)
{
    int i=0,j;
    while ((i<vars->n_var) && (strcmp(vars->names[i],name)!=0)) i++;
    if(i==vars->n_var) //si elle n'existe pas, erreur !
    {
        fprintf(stderr,"Unknown variable \"%s\"\n",name);
        return(NOTYPE);
    }

    ///on parse la chaine afin de detecter le type de variable
    if((strcmp(vars->values[i],"TRUE")==0) || (strcmp(vars->values[i],"true")==0) || (strcmp(vars->values[i],"FALSE")==0) || (strcmp(vars->values[i],"false")==0))
        return(BOOLEAN);
    int n_point = 0;
    for(j=0;j<strlen(vars->values[i]);j++)
    {
        if((vars->values[i][j]!='0') && (vars->values[i][j]!='1') && (vars->values[i][j]!='2') && (vars->values[i][j]!='3') && (vars->values[i][j]!='4')
        && (vars->values[i][j]!='5') && (vars->values[i][j]!='6') && (vars->values[i][j]!='7') && (vars->values[i][j]!='8') && (vars->values[i][j]!='9')
        && (vars->values[i][j]!='.'))
            return(STRING);
        else
        {
            if((vars->values[i][j]=='.'))
                n_point++;
        }
        if(n_point>1)
            return(STRING);
    }
    return(REAL);
}

/*
    Fonction Variables_CommandParser
    Paramètre vars : Pointeur sur objet Variable représentant les variables du shell et leurs valeurs
    Paramètre args : Pointeur sur tableau de chaines représentant les arguments d'une commande
    Remplace chaque appel de variable par sa valeur dans les arguments d'une commande
*/
void Variables_CommandParser(SHELL_Context * context,SHELL_Variables * vars,char ** args)
{
    int i = 0;
    while(args[i]!=NULL) /// ON ANALYSE TOUS LES ARGUMENTS DE LA COMMANDE
    {
        if(strlen(args[i])>1)
        {
            int taille, new_taille = 0, is_var = 0, var_len = 0;
            char * new_arg = (char*)calloc(0,sizeof(char));
            new_arg = NULL;
            char * var_name = NULL, * var_value = NULL;

            /// ON BOUCLE SUR LES CARACTERES DE L'ARGUMENT COURANT
            for(taille=0;taille<strlen(args[i]);taille++)
            {
                /// TEST FIN DE VARIABLE //////////////////////////////////////////////////////////////////////////////////////////////////
                if(( (args[i][taille]=='=') || (args[i][taille]=='-') || (args[i][taille]=='+') || (args[i][taille]=='*') || (args[i][taille]=='/')
                || (args[i][taille]=='\\') || (args[i][taille]=='$') || (args[i][taille]=='(') || (args[i][taille]==')')) && (is_var == 1) )
                {
                    is_var = 0;
                    if(!(var_name = realloc(var_name,(var_len+1)*sizeof(char))))
                    {
                        fprintf(stderr,"Error, too weak memory");
                        exit(EXIT_FAILURE);
                    }
                    var_name[var_len] = '\0';
                    if(strcmp(var_name,"?")==0)
                    {
                        var_value = (char*)calloc(sizeof(char),2);
                        sprintf(var_value,"%d",context->last_return_value);
                    }
                    else if(strcmp(var_name,"§")==0)
                    {
                        var_value = (char*)calloc(sizeof(char),2);
                        sprintf(var_value,"%d",context->n_backgrounds);
                    }
                    else
                        var_value = Variables_GetValue(vars,var_name);
                    if(var_value)
                    {
                        new_taille = new_taille + strlen(var_value);
                        if(!(new_arg = realloc(new_arg,(new_taille)*sizeof(char))))
                        {
                            fprintf(stderr,"Error, too weak memory");
                            exit(EXIT_FAILURE);
                        }
                        if(new_arg)
                            strcat(new_arg,var_value);
                        else
                            strcpy(new_arg,var_value);
                    }
                    else
                    {
                        fprintf(stderr,"Warning : Unknown var \"%s\"\n",var_name);
                        new_taille = new_taille + strlen(var_name)+1;
                        if(!(new_arg = realloc(new_arg,(new_taille)*sizeof(char))))
                        {
                            fprintf(stderr,"Error, too weak memory");
                            exit(EXIT_FAILURE);
                        }
                        if(new_arg)
                            sprintf(new_arg,"%s$%s",new_arg,var_name);
                        else
                            sprintf(new_arg,"$%s",var_name);

                    }
                }

                /// TEST DEBUT DE VARIABLE ///////
                if(args[i][taille]=='$')
                {
                    if(var_name)
                        free(var_name);
                    is_var = 1;
                    var_name = NULL;
                    var_len = 0;
                }

                /// TEST DE SYNTAXE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                else if((is_var == 1) && (var_name==NULL) && ( (args[i][taille]=='0') || (args[i][taille]=='1') || (args[i][taille]=='2') || (args[i][taille]=='3')
                || (args[i][taille]=='4') || (args[i][taille]=='5') || (args[i][taille]=='6') || (args[i][taille]=='7') || (args[i][taille]=='8')
                || (args[i][taille]=='9') ))
                {
                    fprintf(stderr,"Syntaxe error\n");
                    return;
                }

                /// ANALYSE DE LA VARIABLE EN COURS ////////////////////////////
                else if(is_var==1)
                {
                    var_len++;
                    if(!(var_name = realloc(var_name,(var_len)*sizeof(char))))
                    {
                        fprintf(stderr,"Error, too weak memory");
                        exit(EXIT_FAILURE);
                    }
                    var_name[var_len-1] = args[i][taille];
                }

                /// COPIE SIMPLE DU CARATERE COURANT DANS LE NOUVEL ARGUMENT ///
                else
                {
                    new_taille++;
                    if(!(new_arg = realloc(new_arg,(new_taille+1)*sizeof(char))))
                    {
                        fprintf(stderr,"Error, too weak memory");
                        exit(EXIT_FAILURE);
                    }
                    new_arg[new_taille-1] = args[i][taille];
                }
            }

            /// DERNIERE ITERATION, CAS OU UNE VARIABLE EST EN COURS D'ANALYSE MAIS ON ARRIVE A LA FIN DE LA COMMANDE
            if(is_var==1)
            {
                if(!(var_name = realloc(var_name,(var_len+1)*sizeof(char))))
                {
                    fprintf(stderr,"Error, too weak memory");
                    exit(EXIT_FAILURE);
                }
                var_name[var_len] = '\0';
                if(strcmp(var_name,"?")==0)
                {
                    var_value = (char*)calloc(sizeof(char),2);
                    sprintf(var_value,"%d",context->last_return_value);
                }
                else if(strcmp(var_name,"§")==0)
                {
                    var_value = (char*)calloc(sizeof(char),2);
                    sprintf(var_value,"%d",context->n_backgrounds);
                }
                else
                    var_value = Variables_GetValue(vars,var_name);
                if(var_value)
                {
                    new_taille = new_taille + strlen(var_value);
                    if(!(new_arg = realloc(new_arg,(new_taille)*sizeof(char))))
                    {
                        fprintf(stderr,"Error, too weak memory");
                        exit(EXIT_FAILURE);
                    }
                    if(new_arg)
                        strcat(new_arg,var_value);
                    else
                        strcpy(new_arg,var_value);
                }
                else
                {
                    fprintf(stderr,"Warning : Unknown var \"%s\"\n",var_name);
                    new_taille = new_taille + strlen(var_name)+1;
                    if(!(new_arg = realloc(new_arg,(new_taille)*sizeof(char))))
                    {
                        fprintf(stderr,"Error, too weak memory");
                        exit(EXIT_FAILURE);
                    }
                    if(new_arg)
                        sprintf(new_arg,"%s$%s",new_arg,var_name);
                    else
                        sprintf(new_arg,"$%s",var_name);
                }
            }
            /// /////////////////////////////////////////////////////////////////////////////////////////////////////

            /// ON REMPLACE L'ARGUMENT D'ORIGINE PAR L'ARGUMENT TRAITÉ
            free(args[i]);
            args[i] = (char*)calloc(strlen(new_arg)+1,sizeof(char));
            strcpy(args[i],new_arg);
            /// //////////////////////////////////////////////////////
        }
        i++;
    }
}
