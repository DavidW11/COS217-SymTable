all: testsymtablelist testsymtablehash

testsymtablelist: testsymtable.c symtablelist.c
	gcc217 testsymtable.o symtablelist.o -o testsymtablelist

testsymtablehash: testsymtable.c symtablehash.c
	gcc217 testsymtable.o symtablehash.o -o testsymtablehash

testsymtable.o: testsymtable.c symtable.h
	gcc217 -c testsymtable.c

symtablelist.o: symtablelist.c symtable.h
	gcc217 -c symtablelist.c

symtablehash.o: symtablehash.c symtable.h
	gcc217 -c symtablehash.c