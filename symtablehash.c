/*
symtablehash.c
Author: David Wang
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "symtable.h"


/* possible values for the number of buckets in the symbol table */
static const size_t auBucketCounts[] = 
    {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521};
/* number of possible values for the bucket count 
(length of auBucketCounts array) */
static const size_t numBucketCounts = 
    sizeof(auBucketCounts)/sizeof(auBucketCounts[0]);

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


/* A SymTable is a "dummy" node that points to the first SymTableNode.*/
struct SymTable
{
    /* Pointer to the first element of the array of pointers. */
    struct SymTableNode **ppsArray;

    /* The number of bindings in the SymTable */
    size_t length;
    
    /* The index of the current bucket count in the 
        global array of bucket counts*/
    size_t uBucketCountIndex;
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
    const size_t uInitBucketCount = auBucketCounts[0];
    size_t i;

    /* initialize to 509 buckets with each the size of a pointer */
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;
    
    oSymTable->ppsArray = (struct SymTableNode **)
        calloc(uInitBucketCount, sizeof(struct SymTableNode *));
    if (oSymTable->ppsArray == NULL)
        return NULL;
    
    /* intialize all buckets to NULL */
    for (i=0; i<uInitBucketCount; i++) {
        oSymTable->ppsArray[i] = NULL;
    }
    oSymTable->uBucketCountIndex = 0;
    oSymTable->length = 0;
    return oSymTable;
}


/* free memory allocated to the linked list of bindings starting with 
the node pointed to by psFirstNode */
static void SymTable_freeBucket(struct SymTableNode *psFirstNode)
{
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;

    assert(psFirstNode != NULL);

    for (psCurrentNode = psFirstNode;
        psCurrentNode != NULL;
        psCurrentNode = psNextNode)
    {
        psNextNode = psCurrentNode->psNextNode;
        free((char *) psCurrentNode->pcKey);
        free(psCurrentNode);
    }
}


void SymTable_free(SymTable_T oSymTable)
{
    size_t i;
    struct SymTableNode *psCurrentBucket;

    assert(oSymTable != NULL);

    for(i=0; i<auBucketCounts[oSymTable->uBucketCountIndex]; i++) {
        psCurrentBucket = oSymTable->ppsArray[i];
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


/* If the bucket count is not already at the maximum value, expand
oSymTable to the next highest bucket count.
Otherwise, leave oSymTable unchanged. */
static void SymTable_expand(SymTable_T oSymTable) {
    size_t i;
    size_t oldBucketCount;
    size_t newBucketCount;
    struct SymTableNode **ppsNewArray;

    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    size_t uNewHash;

    /* check that bucket count is not at maximum */
    if (oSymTable->uBucketCountIndex == numBucketCounts-1)
        return;
    
    oldBucketCount = auBucketCounts[oSymTable->uBucketCountIndex];
    newBucketCount = auBucketCounts[oSymTable->uBucketCountIndex+1];

    /* allocate memory for newly-sized array of pointers */
    ppsNewArray = (struct SymTableNode **) 
        calloc(newBucketCount, sizeof(struct SymTableNode *));
    if (ppsNewArray == NULL)
        return 0;

    /* initialize newly allocated memory to NULL pointers */
    for(i=0; i<newBucketCount; i++) {
        ppsNewArray[i] = NULL;
    }

    /* put bindings in new array */
    for(i=0; i<oldBucketCount; i++) {
        /* printf("CURRENT BUCKET: %u, ", (unsigned int) i); */
        for(psCurrentNode = oSymTable->ppsArray[i];
            psCurrentNode != NULL;
            psCurrentNode = psNextNode) 
        {
            psNextNode = psCurrentNode->psNextNode;

            uNewHash = SymTable_hash(psCurrentNode->pcKey, 
                newBucketCount);
            /* printf("%u, ", (unsigned int) uNewHash); */
            psCurrentNode->psNextNode = ppsNewArray[uNewHash];
            ppsNewArray[uNewHash] = psCurrentNode;
        }
        oSymTable->ppsArray[i] = NULL;
    }

    /* free pointer to old array, 
    assign new array and bucket count index */
    free(oSymTable->ppsArray);
    oSymTable->ppsArray = ppsNewArray;
    oSymTable->uBucketCountIndex += 1;
}


int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue)
{
    size_t bucketIndex;
    struct SymTableNode *psNewNode;

    assert(oSymTable != NULL);

    /* check if SymTable already contains key */
    if (SymTable_contains(oSymTable, pcKey))
        return 0;
    
    /* expand SymTable if necessary */
    if (oSymTable->length == 
        auBucketCounts[oSymTable->uBucketCountIndex])
        SymTable_expand(oSymTable);
    
    bucketIndex = SymTable_hash(pcKey, 
        auBucketCounts[oSymTable->uBucketCountIndex]);

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
    psNewNode->psNextNode = oSymTable->ppsArray[bucketIndex];
    oSymTable->ppsArray[bucketIndex] = psNewNode;
    /* increment length of SymTable */
    oSymTable->length += 1;
    return 1;
}


/*
return a pointer to the SymTableNode in oSymTable whose key is pcKey. 
If no matching key exists in the symbol table, return NULL.
*/
static struct SymTableNode *SymTable_getNode(SymTable_T oSymTable, 
    const char *pcKey)
{
    struct SymTableNode *psCurrentNode;
    size_t bucketIndex;

    assert(oSymTable != NULL);

    bucketIndex = SymTable_hash(pcKey, 
        auBucketCounts[oSymTable->uBucketCountIndex]);

    for (psCurrentNode = oSymTable->ppsArray[bucketIndex];
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

    bucketIndex = SymTable_hash(pcKey, 
        auBucketCounts[oSymTable->uBucketCountIndex]);

    for (psCurrentNode = oSymTable->ppsArray[bucketIndex];
        psCurrentNode != NULL;
        psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey)==0) {
            if (psPrevNode==NULL) {
                /* condition that we remove the first node */
                pvValue = psCurrentNode->pvValue;
                oSymTable->ppsArray[bucketIndex] = 
                    psCurrentNode->psNextNode;
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


/* 
int SymTable_isEmpty(SymTable_T oSymTable)
{
    size_t i;
    struct SymTableNode *psCurrentBucket;

    assert(oSymTable != NULL);

    for(i=0; i<auBucketCounts[oSymTable->uBucketCountIndex]; i++) {
        psCurrentBucket = oSymTable->ppsArray[i];
        if(psCurrentBucket != NULL)
            return 0;
    }
    return 1;
}
*/


void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra)
{
    size_t i;
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* iterate through buckets */
    for(i=0; i<auBucketCounts[oSymTable->uBucketCountIndex]; i++) {
        /* iterate through linked list */
        for (psCurrentNode = oSymTable->ppsArray[i];
            psCurrentNode != NULL;
            psCurrentNode = psCurrentNode->psNextNode)
        (*pfApply)(psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, 
            (void*)pvExtra);
    }
}
