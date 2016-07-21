CSci4061 S2016 Assignment 1
Wendi An

#make4061
##1.  Purpose
This project is meant to be a simplified version of the regular Linux make command.  It is a program that runs from the UNIX shell.  The project was originally an exercise in learning forks and execs, but I decided to build a tree out of a given make file based on the target's dependencies.  Then the tree is transformed into a stack by a depth-first traversal so that the objects with no dependencies are executed first and so on.

##2.  Compile
To set up make4061, set it up just like you would with Make and enter the following into the terminal:
```
make clean; make
```
##3.  How to Use From Shell
These are several options when using make4061 in the UNIX shell:

| Command in Shell          | Notes                                                          |
|---------------------------|----------------------------------------------------------------|
| ./make4061                | This will build the first target found in makefile            |
| ./make4061 <specific target> | This will build one specific target                      |
| ./make4061 -f <specific name>  | This will make the specified make file           |
| ./make4061 -n             | Displays the commands that would run, but doesn't execute them |
| ./make4061 -m <log file name>    | stores the output into a log file                      |
| ./make4061 -B             | Recompiles without cheking timestamps                        |

##4.Other Things

The program does check timestamps to avoid unecessary recompiling.  If a dependency is older than an existing target for example, it assumes there is no need to recompile it.

________________