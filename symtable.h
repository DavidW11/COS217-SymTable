/*
symtable.h
author: David Wang
*/

#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED
#include <stddef.h>

/* SymTable_T is an unordered collection of key-value bindings with 
no NULL or duplicate keys */
typedef struct SymTable* SymTable_T;

/* Return a new SymTable_T object that contains no 
bindings, or NULL if insufficient memory is available. */
SymTable_T SymTable_new(void);

/* Free all memory occupied by oSymTable. */
void SymTable_free(SymTable_T oSymTable);

/* Return the number of bindings in oSymTable. */
size_t SymTable_getLength(SymTable_T oSymTable);

/*
If oSymTable does not contain a binding with key pcKey, 
add a new binding to oSymTable consisting of 
key pcKey and value pvValue and return 1 (TRUE). 
Otherwise leave oSymTable unchanged and return 
0 (FALSE). If insufficient memory is available,  
leave oSymTable unchanged and return 0 (FALSE).
*/
int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

/* 
If oSymTable contains a binding with key pcKey,
replace the binding's value with pvValue and return the old value. 
Otherwise leave oSymTable unchanged and return NULL.
*/
void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

/*
Return 1 (TRUE) if oSymTable contains a binding 
whose key is pcKey, and 0 (FALSE) otherwise.
*/
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/*
Return the value of the binding within oSymTable 
whose key is pcKey, or NULL if no such binding exists.
*/
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/*
If oSymTable contains a binding with key pcKey, 
remove that binding from oSymTable and return the binding's value. 
Otherwise, do not change oSymTable and return NULL.
*/
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/*
Apply function *pfApply to each binding in oSymTable, 
passing pvExtra as an extra parameter.
*/
void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra);

#endif