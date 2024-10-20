/*
symtablelist.c
Author: David Wang
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "symtable.h"

/*
assert statements to cover keys and values
comments
*/

/* Each item is stored in a SymTableNode.  SymTableNodes are linked to
   form a list.  */
struct SymTableNode
{
    /* pointer to the value. */
    const void *pvValue;

    /* pointer to defensive copy of the key string */
    const char *pcKey;

    /* The address of the next SymTableNode. */
    struct SymTableNode *psNextNode;
};


/* A SymTable is a "dummy" node that points to the first SymTableNode. */
struct SymTable
{
    /* The address of the first SymTableNode. */
    struct SymTableNode *psFirstNode;

    /* The number of bindings in the SymTable */
    size_t length;
};


SymTable_T SymTable_new(void)
{
    SymTable_T oSymTable;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;

    oSymTable->psFirstNode = NULL;
    oSymTable->length = 0;
    return oSymTable;
}


void SymTable_free(SymTable_T oSymTable)
{
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;

    assert(oSymTable != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psNextNode)
    {
        psNextNode = psCurrentNode->psNextNode;
        free((char *) psCurrentNode->pcKey);
        free(psCurrentNode);
    }

    free(oSymTable);
}


size_t SymTable_getLength(SymTable_T oSymTable) {
    return oSymTable->length;
}


int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue)
{
    struct SymTableNode *psNewNode;

    assert(oSymTable != NULL);

    /* check if SymTable already contains key */
    if (SymTable_contains(oSymTable, pcKey))
        return 0;
    
    psNewNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
    if (psNewNode == NULL)
        return 0;

    /* create defensive copy of key */
    psNewNode->pcKey = (const char*)malloc(strlen(pcKey)+1);
    if (psNewNode->pcKey == NULL)
        return 0;
    strcpy((char *) psNewNode->pcKey, pcKey);

    /* assign value */
    psNewNode->pvValue = pvValue;

    /* insert binding to beginning of linked list */
    psNewNode->psNextNode = oSymTable->psFirstNode;
    oSymTable->psFirstNode = psNewNode;
    /* increment length of SymTable */
    oSymTable->length += 1;
    return 1;
}

/*
helper function ...
return a pointer to the symbol table node with the same key, 
or NULL if such a key does not exist in the symbol table.
*/
struct SymTableNode *getSymTableNode(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode)
    {   
        if (strcmp(psCurrentNode->pcKey, pcKey)==0)
            return psCurrentNode;
    }
    return NULL;
}


void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue)
{
    struct SymTableNode *node;
    const void *pvOldValue;

    assert(oSymTable != NULL);

    node = getSymTableNode(oSymTable, pcKey);
    if (node==NULL) {
        return NULL;
    }
    pvOldValue = node->pvValue;
    node->pvValue = pvValue;
    return (void *) pvOldValue;
}


int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *node;

    assert(oSymTable != NULL);

    node = getSymTableNode(oSymTable, pcKey);
    if (node==NULL) {
        return 0;
    }
    return 1;
}


void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *node;

    assert(oSymTable != NULL);

    node = getSymTableNode(oSymTable, pcKey);
    if (node==NULL) {
        return NULL;
    }
    return (void *) node->pvValue;
}


void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *psPrevNode = NULL;
    struct SymTableNode *psCurrentNode;
    const void *pvValue;

    assert(oSymTable != NULL);
    assert(oSymTable->psFirstNode != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey)==0) {
            if (psPrevNode==NULL) {
                /* condition that we remove the first node */
                pvValue = psCurrentNode->pvValue;
                oSymTable->psFirstNode = psCurrentNode->psNextNode;
            }
            else {
                pvValue = psCurrentNode->pvValue;
                psPrevNode->psNextNode = psCurrentNode->psNextNode;
            }
            free((char *) psCurrentNode->pcKey);
            free(psCurrentNode);
            /* decrement length of SymTable */
            oSymTable->length -= 1;
            return (void *) pvValue;
        }
        psPrevNode = psCurrentNode;
    }
    return NULL;
}


int SymTable_isEmpty(SymTable_T oSymTable)
{
   assert(oSymTable != NULL);

   return oSymTable->psFirstNode == NULL;
}


void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra)
{
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode)
        (*pfApply)(psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, 
            (void*)pvExtra);
}
