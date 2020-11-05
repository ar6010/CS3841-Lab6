#include <stdio.h>
#include <stdlib.h>
#include "memory_manager.h"

/* 
 * Using static causes the compiler to
 * limit visibility of the varibles to this file only
 * This can be used to simulate 'private' variables in c
 */
static int allocation_count = 0;
static int *endPointer;

//linked list structure
static struct memoryBlock {
    void *blockAddress;
    struct memoryBlock *next;
    int isFree;
    size_t size;
} memoryBlock;

static struct memoryBlock *first;
static struct memoryBlock *last;

/* TODO Define additional structure definitions here */

/* mmInit()
 *     Initialize the memory manager to "manage" the given location
 *     in memory with the specified size.
 *         Parameters: start - the start of the memory to manage
 *                     size - the size of the memory to manage
 *         Returns: void
 */
void mmInit(void* start, int size)
{
    allocation_count = 0;
    endPointer = start + size;
    //Initialize first block
    first->blockAddress = start;
    first->next = NULL;
    first->isFree = 1;
    first->size = size;
    last = first;
    // TODO more initialization needed
}

/* mmDestroy()
 *     Cleans up any storage used by the memory manager
 *     After a call to mmDestroy:
 *         all allocated spaces become invalid
 *         future allocation attempts fail
 *         future frees result in segmentation faults
 *     NOTE: after a call to mmDestroy a call to mmInit
 *           reinitializes the memory manager to allow allocations
 *           and frees
 *         Parameters: None
 *         Returns: void
 */
void mmDestroy()
{
    //all allocated spaces become invalid
    // future allocation attempts fail
    // future frees result in segmentation faults
    if(first == NULL){
        printf("Must initialize Memory Manager");
    } else{
        struct memoryBlock *tempBlock;
        struct memoryBlock *currentBlock;
        tempBlock = first;
        while(tempBlock != NULL){
            currentBlock = tempBlock;
            free(tempBlock);
            tempBlock = currentBlock->next;
        }
    }
}

/* mymalloc_ff()
 *     Requests a block of memory be allocated using
 *         first fit placement algorithm
 *     The memory manager must be initialized (mmInit)
 *         for this call to succeed
 *         Parameters: nbytes - the number of bytes in the requested memory
 *         Returns: void* - a pointer to the start of the allocated space
 */
void* mymalloc_ff(int nbytes)
{
    if(first == NULL){
        printf("Must initialize Memory Manager");
        return NULL;
    }
    struct memoryBlock *newBlock = (struct memoryBlock*)malloc(sizeof(struct memoryBlock));
    newBlock->size = nbytes;
    newBlock->isFree = 0;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if(newBlock->size <= tempBlock->size){
            break;
        }
        tempBlock = tempBlock->next;
    }
    if(tempBlock != NULL){
        newBlock->blockAddress = tempBlock->blockAddress;
        tempBlock->size -= newBlock->size;
        if(first == NULL){
            first = newBlock;
        }
        else{
            last = first;
            while(last->next != NULL){
                last = last->next;
            }
            last->next = newBlock;
        }
        allocation_count++;
        return newBlock->blockAddress;
    } else{
        printf("Can't allocate block of this size");
    }
    return NULL;
}

/* mymalloc_wf()
 *     Requests a block of memory be allocated using
 *         worst fit placement algorithm
 *     The memory manager must be initialized (mmInit)
 *         for this call to succeed
 *         Parameters: nbytes - the number of bytes in the requested memory
 *         Returns: void* - a pointer to the start of the allocated space
 */
void* mymalloc_wf(int nbytes)
{
    if(first == NULL){
        printf("Must initialize Memory Manager");
        return NULL;
    }

    //Find largest memory block
    int max = 0;
    void* maxAddress = 0;
    struct memoryBlock *temp = first;
    while(temp->next != NULL){
        if(temp->size > max && temp->isFree == 0){
            max = temp->size;
            maxAddress = temp->blockAddress;
        }
        *temp = *temp->next;
    }

    //Return address of largest memory block if it is larger than nbytes, else create a new block
    if(max > (last->next - *endPointer) && max >= nbytes){
        return maxAddress;
    } else if(nbytes < (last->next - *endPointer)){
        struct memoryBlock *newBlock = (struct memoryBlock*)malloc(sizeof(struct memoryBlock));
        newBlock->size = nbytes;
        newBlock->isFree = 0;
        newBlock->blockAddress = last->blockAddress;
        newBlock->next = newBlock->blockAddress + nbytes;
        last->blockAddress = newBlock->next;
        allocation_count++;
        return newBlock->blockAddress;
    }


    return NULL;
}

/* mymalloc_bf()
 *     Requests a block of memory be allocated using
 *         best fit placement algorithm
 *     The memory manager must be initialized (mmInit)
 *         for this call to succeed
 *         Parameters: nbytes - the number of bytes in the requested memory
 *         Returns: void* - a pointer to the start of the allocated space
 */
void* mymalloc_bf(int nbytes)
{
    if(first == NULL){
        printf("Must initialize Memory Manager");
        return NULL;
    }

    //Find best memory block using a calculated fit value
    int fit = nbytes;
    void* fitAddress;
    struct memoryBlock *temp = first;
    while(temp->next != NULL){
        if(temp->size >= nbytes && temp->isFree == 0){
            int fitValue = temp->size - nbytes;
            if(fitValue >= 0 && fitValue < fit){
                fitAddress = temp->blockAddress;
            }

        }
        *temp = *temp->next;
    }

    //Return address of largest memory block if it is larger than nbytes, else create a new block
    if(fitAddress != NULL){
        return fitAddress;
    } else if(nbytes < (last->next - *endPointer)){
        struct memoryBlock *newBlock = (struct memoryBlock*)malloc(sizeof(struct memoryBlock));
        newBlock->size = nbytes;
        newBlock->isFree = 0;
        newBlock->blockAddress = last->blockAddress;
        newBlock->next = newBlock->blockAddress + nbytes;
        last->blockAddress = newBlock->next;
        allocation_count++;
        return newBlock->blockAddress;
    }


    return NULL;
}

/* myfree()
 *     Requests a block of memory be freed and the storage made
 *         available for future allocations
 *     The memory manager must be initialized (mmInit)
 *         for this call to succeed
 *         Parameters: ptr - a pointer to the start of the space to be freed
 *         Returns: void
 *         Signals a SIGSEGV if a free is not valid
 *             - memory manager is not initialized
 *             - memory manager has been destroyed
 *             - ptr is not allocated (e.g. double free)
 */
void myfree(void* ptr)
{
    if(first == NULL){
        printf("Must initialize Memory Manager");
    } else if (ptr == NULL){
        printf("Block is not allocated");
    } else {
        struct memoryBlock *tempBlock;
        tempBlock = first;
        while(tempBlock->blockAddress != ptr){
            tempBlock = tempBlock->next;
        }
        tempBlock->isFree = 1;
    }
}

/* get_allocated_space()
 *     Retrieve the current amount of space allocated by the memory manager (in bytes)
 *         Parameters: None
 *         Returns: int - the current number of allocated bytes
 */
int get_allocated_space()
{
    int sum = 0;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if(tempBlock->isFree == 0){
            sum = sum + tempBlock->size;
        }
        tempBlock = tempBlock->next;
    }
    return sum;
}

/* get_remaining_space()
 *     Retrieve the current amount of available space in the memory manager (in bytes)
 *         (e.g. sum of all free blocks)
 *         Parameters: None
 *         Returns: int - the current number of free bytes
 */
int get_remaining_space()
{
    int sum = 0;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if(tempBlock->isFree == 1){
            sum = sum + tempBlock->size;
        }
        tempBlock = tempBlock->next;
    }
    return sum;
}

/* get_fragment_count()
 *     Retrieve the current amount of free blocks (i.e. the count of all the block, not the size)
 *         Parameters: None
 *         Returns: int - the current number of free blocks
 */
int get_fragment_count()
{
    int sum = 0;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if(tempBlock->isFree == 1){
            sum = sum + 1;
        }
        tempBlock = tempBlock->next;
    }
    return sum;
}

/* get_mymalloc_count()
 *     Retrieve the number of successfull malloc calls (for all placement types)
 *         Parameters: None
 *         Returns: int - the total number of successfull mallocs
 */
int get_mymalloc_count()
{
    return allocation_count;
}
