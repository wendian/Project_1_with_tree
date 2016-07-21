/************************
 * util.c
 *
 * utility functions (first half provided by class)
 *
 ************************/

#include "util.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/***************
 * These functions are just some handy file functions.
 * We have not yet covered opening and reading from files in C,
 * so we're saving you the pain of dealing with it, for now.
 *******/
FILE* file_open(char* filename)
{
	FILE* fp = fopen(filename, "r");
	if(fp == NULL)
   {
		fprintf(stderr, "make4061: %s: No such file or directory.\n", filename);
		exit(1);
	}
	return fp;
}

char* file_getline(char* buffer, FILE* fp) 
{
	buffer = fgets(buffer, 1024, fp);
	return buffer;
}

//Return -1 if file does not exist
int is_file_exist(char * lpszFileName)
{
	return access(lpszFileName, F_OK); 
}

int get_file_modification_time(char * lpszFileName)
{
	if(is_file_exist(lpszFileName) != -1)
	{
		struct stat buf;
		int nStat = stat(lpszFileName, &buf);
		return buf.st_mtime;
	}
	return -1;
}

int compare_modification_time(char * lpsz1, char * lpsz2)
{	
	int nTime1 = get_file_modification_time(lpsz1);
	int nTime2 = get_file_modification_time(lpsz2);
	if(nTime1 == -1 || nTime2 == -1) return -1;
	if(nTime1 == nTime2) return 0;
	else if(nTime1 > nTime2) return 1;
	else return 2;
}

/* Taken from Unix Systems Programming, Robbins & Robbins, p37 */
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }

   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

void freemakeargv(char **argv)
{
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

// All the following functions were ritten by me

int push(struct stack * top, int i)
{
   struct stack * t = (struct stack *) malloc(sizeof(struct stack));
   t->index = i;
   t->next = top->next;
   top->next = t;
   return 0;
}

int pop(struct stack * top)
{
   struct stack * t = top->next;
   top->next = t->next;
   int n = t->index;
   free(t);
   return n;
}

int is_empty(struct stack * top)
{
   return top->next == NULL;
}

void construct(struct stack * top, struct t_node * root)
{
   if (root != NULL)
   {
      if (root->nStatus == READY) push(top, root->index);
      int i;
      for (i = 0; i < root->nDeps; i++)
      {
         construct(top, root->aDep[i]);
      }
   }
}

struct stack * build_stack(struct t_node * root)
{
   struct stack * top = (struct stack *) malloc(sizeof(struct stack));
   top->next = NULL;
   top->index = -1;
   construct(top, root);
   return top;
}

int init_tree(struct t_node * t)
{
   int i;
   for (i = 0; i < MAX_CHILDREN; i++)
   {
      t->aDep[i] = NULL;
   }
   return 0;
}

int add(struct t_node * root, struct t_node * t)
{
   int i;
   for (i = 0; i < root->nDeps; i++)
   {
      if (strcmp(t->szTarget, root->szDeps[i]) == 0)
      {
         root->aDep[i] = t;
         return 0;
      }
      if (root->aDep[i] != NULL)
      {
         if (add(root->aDep[i], t) == 0) return 0;
      }
   }
   return -1;
}
