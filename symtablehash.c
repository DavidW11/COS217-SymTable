/*
symtablehash.c
Author: David Wang
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "symtable.h"


/* Each item is stored in a SymTableNode. SymTableNodes are linked to
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
    /* Pointer to the first element of the array of pointers. */
    struct SymTableNode **ppsArray;

    /* The number of bindings in the SymTable */
    size_t length;

    /* The number of buckets in the SymTable */
    size_t uBucketCount;
};


/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % uBucketCount;
}


SymTable_T SymTable_new(void)
{
    SymTable_T oSymTable;

    /* initialize to 509 buckets with each the size of a pointer */
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;
    
    oSymTable->ppsArray = (struct SymTableNode **)
        calloc(509, sizeof(struct SymTableNode *));
    if (oSymTable->ppsArray == NULL)
        return NULL;
    
    oSymTable->uBucketCount = 509;
    oSymTable->length = 0;
    return oSymTable;
}


void SymTable_freeBucket(struct SymTableNode *psFirstNode)
{
    struct SymTableNode *psCurrentNode;

    assert(psFirstNode != NULL);

    for (psCurrentNode = psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode)
    {
        free((char *) psCurrentNode->pcKey);
        free(psCurrentNode);
    }
}


void SymTable_free(SymTable_T oSymTable)
{
    size_t i;
    struct SymTableNode *psCurrentBucket;

    assert(oSymTable != NULL);

    for(i=0; i<oSymTable->uBucketCount; i++) {
        psCurrentBucket = *(oSymTable->ppsArray) + i;
        if(psCurrentBucket == NULL)
            continue;
        SymTable_freeBucket(psCurrentBucket);
    }

    free(oSymTable->ppsArray);
    free(oSymTable);
}


size_t SymTable_getLength(SymTable_T oSymTable) {
    return oSymTable->length;
}


int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue)
{
    /* expansion code here */

    size_t uBucketCount;
    size_t bucketIndex;
    struct SymTableNode *psNewNode;
    struct SymTableNode *psFirstNode;

    assert(oSymTable != NULL);
    
    uBucketCount = oSymTable->uBucketCount;
    bucketIndex = SymTable_hash(pcKey, uBucketCount);
    psFirstNode = *(oSymTable->ppsArray)+bucketIndex;

    /* check if SymTable already contains key */
    if (SymTable_contains(oSymTable, pcKey))
        return 0;

    psNewNode = (struct SymTableNode*)
        malloc(sizeof(struct SymTableNode));
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
    psNewNode->psNextNode = psFirstNode;
    psFirstNode = psNewNode;
    /* increment length of SymTable */
    oSymTable->length += 1;
    return 1;
}


/*
helper function ...
return a pointer to the symbol table node with the same key, 
or NULL if such a key does not exist in the symbol table.
*/
struct SymTableNode *SymTable_getNode(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *psCurrentNode;
    size_t bucketIndex;

    assert(oSymTable != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->uBucketCount);

    for (psCurrentNode = *(oSymTable->ppsArray)+bucketIndex;
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

    node = SymTable_getNode(oSymTable, pcKey);
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

    node = SymTable_getNode(oSymTable, pcKey);
    if (node==NULL) {
        return 0;
    }
    return 1;
}


void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *node;

    assert(oSymTable != NULL);

    node = SymTable_getNode(oSymTable, pcKey);
    if (node==NULL) {
        return NULL;
    }
    return (void *) node->pvValue;
}


void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *psPrevNode = NULL;
    struct SymTableNode *psCurrentNode;
    size_t bucketIndex;
    const void *pvValue;

    assert(oSymTable != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->uBucketCount);

    for (psCurrentNode = *(oSymTable->ppsArray)+bucketIndex;
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey)==0) {
            if (psPrevNode==NULL) {
                /* condition that we remove the first node */
                pvValue = psCurrentNode->pvValue;
                psCurrentNode = psCurrentNode->psNextNode;
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
    size_t i;
    struct SymTableNode *psCurrentBucket;

    assert(oSymTable != NULL);

    for(i=0; i<oSymTable->uBucketCount; i++) {
        psCurrentBucket = *(oSymTable->ppsArray) + i;
        if(psCurrentBucket != NULL)
            return 0;
    }
    return 1;
}


void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra)
{
    size_t i;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* iterate through buckets */
    for(i=0; i<oSymTable->uBucketCount; i++) {
        /* iterate through linked list */
        for (psCurrentNode = *(oSymTable->ppsArray) + i;
            psCurrentNode != NULL;
            psCurrentNode = psCurrentNode->psNextNode)
        (*pfApply)(psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, 
            (void*)pvExtra);
    }
}
