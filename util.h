/********************
 * util.h
 *
 * You may put your utility function definitions here
 * also your structs, if you create any
 *********************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

// the following ifdef/def pair prevents us from having problems if 
// we've included util.h in multiple places... it's a handy trick
#ifndef _UTIL_H_
#define _UTIL_H_

#define INELIGIBLE 0
#define READY 1

#define MAX_LENGTH 1024
#define MAX_CHILDREN 10
#define MAX_NODES 10

// This stuff is for easy file reading
FILE * file_open(char*);
char * file_getline(char*, FILE*);
int is_file_exist(char *);
int get_file_modification_time(char *);
int compare_modification_time(char *, char *);

struct stack
{
	int index;
	struct stack * next;
};

struct t_node
{
	int index;
	int nStatus;
	int nDeps;
	char szDeps[MAX_CHILDREN][64];
	char szCommand[64];
	char szTarget[64];
	struct t_node * aDep[MAX_CHILDREN];
};

int push(struct stack * top, int i);
int pop(struct stack * top);
int is_empty(struct stack * top);

void construct(struct stack * top, struct t_node * root);
struct stack * build_stack(struct t_node * root);

int init_tree(struct t_node * t);
int add(struct t_node * root, struct t_node * t);

int makeargv(const char *s, const char *delimiters, char ***argvp);
void freemakeargv(char **argv);

#endif
