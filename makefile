all: testsymtablelist testsymtablehash

testsymtablelist: testsymtable.o symtablelist.o
	gcc217m -g testsymtable.o symtablelist.o -o testsymtablelist

testsymtablehash: testsymtable.o symtablehash.o
	gcc217m -g testsymtable.o symtablehash.o -o testsymtablehash

testsymtable.o: testsymtable.c symtable.h
	gcc217m -g -c testsymtable.c

symtablelist.o: symtablelist.c symtable.h
	gcc217m -g -c symtablelist.c

symtablehash.o: symtablehash.c symtable.h
	gcc217m -g -c symtablehash.c