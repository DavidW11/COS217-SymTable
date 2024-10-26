all: testsymtablelist testsymtablehash testsymtablelistm testsymtablehashm

clobber: clean
	rm -f *~\#*\#

clean: 
	rm -f testsymtablelist* testsymtablehash* *.o

testsymtablelist: testsymtable.o symtablelist.o
	gcc217 testsymtable.o symtablelist.o -o testsymtablelist

testsymtablehash: testsymtable.o symtablehash.o
	gcc217 testsymtable.o symtablehash.o -o testsymtablehash

testsymtable.o: testsymtable.c symtable.h
	gcc217 -c testsymtable.c

symtablelist.o: symtablelist.c symtable.h
	gcc217 -c symtablelist.c

symtablehash.o: symtablehash.c symtable.h
	gcc217 -c symtablehash.c


testsymtablelistm: testsymtablem.o symtablelistm.o
	gcc217m -g testsymtablem.o symtablelistm.o -o testsymtablelistm

testsymtablehashm: testsymtablem.o symtablehashm.o
	gcc217m -g testsymtablem.o symtablehashm.o -o testsymtablehashm

testsymtablem.o: testsymtable.c symtable.h
	gcc217m -g -c testsymtable.c -o testsymtablem.o

symtablelistm.o: symtablelist.c symtable.h
	gcc217m -g -c symtablelist.c -o symtablelistm.o

symtablehashm.o: symtablehash.c symtable.h
	gcc217m -g -c symtablehash.c -o symtablehashm.o