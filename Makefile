#Ben and Wendi's makefile

make4061: main.c util.c
	gcc -o make4061 main.c util.c
clean:
	rm make4061 makeargv.o

