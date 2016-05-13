/*
    Fichier tools.c : Contient les fonctions outils fréquemment utilisées dans le programme
    Auteurs : Fabien BASSET, Clément PALLUEL
    Dépendances : tools.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

/*
    Fonction Read_Input
    Paramètre MAX_BUF_SIZE : Entier représentant la taille maximum de la chaine saisie
    Lit une chaine de caractères sur l'entrée standart
    Retourne un pointeur sur chaine de caractères contenant la ligne saisie par l'utilisateur
*/
char * Read_Input(int MAX_BUF_SIZE)
{
	if(MAX_BUF_SIZE<0)
		MAX_BUF_SIZE = NO_MAX_BUF;
	int size = 0, max = (MAX_BUF_SIZE==NO_MAX_BUF);
	char c, *res = NULL;
	while(((size<MAX_BUF_SIZE) || (max)) && ((c = getc(stdin))!= '\n') && (c!=EOF))
	{
		if(!(res = realloc(res,sizeof(char)*(size+1)))) { perror("Memory Allocation"); exit(EXIT_FAILURE); }
		res[size++] = c;
	}
	if(!(res = realloc(res,sizeof(char)*(size+1)))) { perror("Memory Allocation"); exit(EXIT_FAILURE); }
		res[size] = '\0';
	return(res);
}

/*
    Fonction Cut_String
    Paramètre str : Pointeur sur chaine représentant la chaine a découper
    Paramètre d_block : Pointeur sur chaine représentant le bloc de caratères délimitateur pour le découpage
    Retourne un pointeur sur tableau de chaines
*/
char ** Cut_String(char * str,const char * d_block)
{
	char**res = res = calloc(1,sizeof(char*));
	res[0] = NULL;
	int i = 0, j, found, r_size = 1, cursor = 1, size1 = strlen(str), size2 = strlen(d_block);
	while(i < size1)
	{
		found = 1;
		for(j=0;(j<size2)&&(j+i<size1)&&(found==1);j++)
			if(str[i+j]!=d_block[j])
				found = 0;
		if((found==1)  && (j==size2))
		{
			if(i+j==size1)
				break;
			if(res[r_size-1])
			{
				if(!(res = realloc(res,(++r_size)*sizeof(char*)))) { perror("Memory Allocation"); exit(-1); }
				res[r_size-1] = NULL;
				cursor = 1;
			}
			i+=j;
		}
		else
		{
			if(!(res[r_size-1] = realloc(res[r_size-1],++cursor*sizeof(char)))) { perror("Memory Allocation"); exit(-1); }
			res[r_size-1][cursor-2] = str[i++];
			res[r_size-1][cursor-1] = '\0';
		}
	}
	if(!(res = realloc(res,(++r_size)*sizeof(char*)))) { perror("Memory Allocation"); exit(-1); }
	res[r_size-1] = NULL;
	return(res);
}

/*
    Fonction Array_Free
    Paramètre array : Pointeur sur tableau à libérer
    Paramètre n_dim : Nombre de dimensions du tableau
    Libère la mémoire allouée d'un tableau de n'importe quel type primaire à n_dim dimensions
*/
void Array_Free(void * array,int n_dim)
{
	if(!array) { fprintf(stderr,"Array_Free : Cannot free NULL pointer. Abort.\n");	return; }
	if(n_dim>1)
	{
		int i=0;
		size_t size = sizeof(void *);
		while(*(void**)(array + i*size) )
			Array_Free(*(void**)(array+i++*size), n_dim-1);
	}
	free(array);
}
