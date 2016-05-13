#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOTYPE -1
#define REAL 0
#define STRING 1
#define BOOLEAN 2

#ifndef _SHELL_VARS
#define _SHELL_VARS
#endif // SHELL_VARS

typedef struct
{
    char ** names; /* noms des variables */
    char ** values; /* valeurs des variables */
    int n_var; /* nombres de variables */
}
SHELL_Variables;

typedef struct
{
    /* traitement des commandes */
    char ** commands; /* tableau contenant les commandes */
    int background; /* booléan avant ou arriere plan */
    int n_commands; /* nombre total de commandes dans le tableau */

    /* variables globales du shell */
    int last_return_value; /* derneiere valeur retournée par une commande */
    int n_backgrounds; /* nombre de commandes en arriere plan */
    SHELL_Variables * vars; /* variables du shell */
}
SHELL_Context;

SHELL_Variables * Variables_Create();

void Free_Variables(SHELL_Variables * vars);

void Variables_Add(SHELL_Variables * vars,char * name,char * value);

int Variables_Remove(SHELL_Variables ** vars,char * name);

char * Variables_GetValue(SHELL_Variables * vars,char * name);

int Variables_SetValue(SHELL_Variables * vars,char * name,char * value);

int Variables_GetType(SHELL_Variables * vars,char * name);

void Variables_CommandParser(SHELL_Context * context,SHELL_Variables * vars,char ** args);
