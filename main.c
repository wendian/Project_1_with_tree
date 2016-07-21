#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"

#define MAX_TARGETS 64

int print = 0;
int check = 1;
int toLog = 0;
int nTargets = 0;
char szMakefile[64] = "Makefile";
struct t_node all[MAX_TARGETS];

int find_index(char * name)
{
	int i;
	for (i = 0; i < MAX_TARGETS; i++)
	{
		if (strcmp(all[i].szTarget, name) == 0) return i;
	}
	return -1;
}

//This function will parse makefile input from user or default makefile. 
int parse(char * lpszFileName)
{
	int nLine = 0;
	int i = 0;
	char szLine[1024];
	char * lpszLine = NULL;
	char * depend;
	FILE * fp = file_open(lpszFileName);
	if(fp == NULL)
	{
		printf("File not found\n");
		return -1;
	}
	while(file_getline(szLine, fp) != NULL) 
	{
		nLine++;
		//Skip newlines and comments
		if (* szLine == '\n' || * szLine == '#') continue;
		lpszLine = strtok(szLine, "\n");
		//Remove leading whitespace
		while (* lpszLine == ' ') lpszLine++;
		//Skip if whitespace-only.
		if (strlen(lpszLine) < 1) continue;
		//Error if line starts with colon
		if (* lpszLine == ':')
		{
			printf("No target found for line %d\n", nLine);
			return -1;
		}
		//If lpszLine starts with '\t' it will be a command else it will be target.
		all[i].nStatus = INELIGIBLE;
		if (* lpszLine == '\t')
		{
			lpszLine++;
			if (all[i - 1].nStatus == READY) strcpy(all[i - 1].szCommand, lpszLine);
			else
			{
				printf("No target found for command on line %d\n", nLine);
				return -1;
			}
		}
		else
		{
			all[i].nStatus = READY;
			all[i].index = i;
			strcpy(all[i].szTarget, strtok(lpszLine, ":"));
			all[i].nDeps = 0;
			depend = strtok(NULL, " ");
			while (depend != NULL)
			{
				strcpy(all[i].szDeps[all[i].nDeps], depend);
				all[i].nDeps++;
				depend =strtok(NULL, " ");
			}
			i++;
		}
		//It is possbile that target may not have a command as you can see from the example on project write-up. (target:all
		//If you found any syntax error, stop parsing.
	}
	nTargets = i;
	fclose(fp);
	return 0;
}

void show_error_message(char * lpszFileName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
	fprintf(stderr, "-m FILE\t\tRedirect the output to the file specified .\n");
	exit(0);
}

int needs_recom(struct t_node * t)
{
	if (!check || (is_file_exist(t->szTarget) == -1)) return 1; //if -B flag, always recompile
	else
	{
		if (t->nDeps != 0) //if file exists and has no dependencies, don't recompile it
		{
			int dep = t->nDeps;
			int i;
			for (i = 0; i < dep; i++)
			{
				if (is_file_exist(t->szDeps[i]) == -1) return 1; // if the any of the file's dependencies do not exist, recompile it
			}
			//if the target dependecies exists, iterate through its dependencies
			int times;
			for (i = 0; i < dep; i++)
			{
				times = compare_modification_time(t->szDeps[i], t->szTarget);
				if (times == 1) return 1; //if dependecy is newer than target, recompile
				else if (times != 2 && times != 0)
				{
					printf("Couldn't compare times");
					exit(-1);
				}
			}
		}
		return 0;
	}
}

void build_tree(struct t_node * root, struct t_node * t)
{
	int i, n;
	struct t_node * child;
	for (i = 0; i < t->nDeps; i++)
	{
		n = find_index(t->szDeps[i]);
		if (n > 0)
		{
			child = &all[n];
			init_tree(child);
			add(root, child);
			build_tree(root, child);
		}
		else if (n < 0 && (is_file_exist(t->szDeps[i]) == -1))
			fprintf(stderr, "make4061: *** No rule to make target `%s', needed by `%s'.  Stop.\n", t->szDeps[i], t->szTarget);
	}
}

void exe_stack(struct stack * s)
{
	char * here;
	pid_t child_pid;
	char ** argv_child;
	int i;
	while (!is_empty(s))
	{
		i = pop(s);
		child_pid = fork();
		if (child_pid == 0)
		{
			if (needs_recom(&all[i]))
			{
				printf("%s\n", all[i].szCommand);
				fflush(stdout);
				if (!print)
				{
					makeargv(all[i].szCommand, " ", &argv_child);
					execvp(*argv_child, argv_child);
				}
			}
			exit(0);
		}
		else if (child_pid == -1)
		{
			printf("Failed to fork\n");
			exit(-1);
		}
		else wait(NULL);
	}
}

int main(int argc, char **argv)
{
	extern int optind;
	extern char * optarg;
	int ch, i;
	char * format = "f:hnBm:";	
	char szTarget[64];
	char szLog[64];

	while((ch = getopt(argc, argv, format)) != -1) 
	{
		//switch statment that handles the flags
		switch(ch) 
		{
			case 'f':
				strcpy(szMakefile, strdup(optarg));
				break;
			case 'n':
				print = 1;
				check = 0;
				break;
			case 'B':
				check = 0;
				break;
			case 'm':
				strcpy(szLog, strdup(optarg));
				toLog = 1;
				break;
			case 'h':
			default:
				show_error_message(argv[0]);
				exit(1);
		}
	}
	//dup2, if -B is set, redirect all output to file
	int out;	
	if(toLog)
	{
		out = open(szLog, O_RDWR | O_TRUNC | O_CREAT, 0666);
		if(out == -1)
		{
			printf("Failed to open output.");
			return EXIT_FAILURE;
		}
		if (dup2(out, STDOUT_FILENO) == -1) 
		{
			printf("Failed to redirect stdout.");
			return EXIT_FAILURE;
		}
	}
	argc -= optind;
	argv += optind;

	if(argc > 1)
	{
		show_error_message(argv[0]);
		return EXIT_FAILURE;
	}
	/* Parse graph file or die */
	if((parse(szMakefile)) == -1)
	{
		printf("Failed to parse file\n");
		return EXIT_FAILURE;
	}
	//if target is not set, set it to the first target from makefile
	int t = 0;
	if(argc == 1)
	{
		for (i = 0; i < MAX_TARGETS; i++)
		{
			if (strcmp(argv[0], all[i].szTarget) == 0)
			{
				t = i;
				break;
			}
		}
		if (t == 0)
		{
			//if given target is not found, exit
			printf("*** No rule to make given target.  Stop.\n");
			return EXIT_FAILURE;
		}
	}
	
	struct t_node * root = &all[t];
	init_tree(root);
	build_tree(root, root);
	struct stack * s = build_stack(root);
	exe_stack(s);

	if(toLog) close(out);
	return EXIT_SUCCESS;
}
