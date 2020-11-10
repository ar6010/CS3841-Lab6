#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
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

sem_t memorySem;

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
    sem_init(&memorySem, 0, 1);
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
    sem_wait(&memorySem);
    struct memoryBlock *newBlock = (struct memoryBlock*)malloc(sizeof(struct memoryBlock));
    newBlock->size = nbytes;
    newBlock->isFree = 0;
    newBlock->next = NULL;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if((newBlock->size <= tempBlock->size) && (tempBlock->isFree)){
            break;
        }
        tempBlock = tempBlock->next;
    }
    if(tempBlock != NULL){
        newBlock->blockAddress = tempBlock->blockAddress;
        tempBlock->size -= newBlock->size;
        tempBlock->blockAddress = tempBlock->blockAddress + nbytes;
        newBlock->next = tempBlock;
        last = first;
        while(last->next->isFree != 1){
            last = last->next;
        }
        last->next = newBlock;
        allocation_count++;
        sem_post(&memorySem);
        return newBlock->blockAddress;
    } else{
        printf("Can't allocate block of this size");
        sem_post(&memorySem);
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
    sem_wait(&memorySem);
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
        sem_post(&memorySem);
        return maxAddress;
    } else if(nbytes < (last->next - *endPointer)){
        struct memoryBlock *newBlock = (struct memoryBlock*)malloc(sizeof(struct memoryBlock));
        newBlock->size = nbytes;
        newBlock->isFree = 0;
        newBlock->blockAddress = last->blockAddress;
        newBlock->next = newBlock->blockAddress + nbytes;
        last->blockAddress = newBlock->next;
        allocation_count++;
        sem_post(&memorySem);
        return newBlock->blockAddress;
    }
    sem_post(&memorySem);
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
    sem_wait(&memorySem);
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
        sem_post(&memorySem);
        return fitAddress;
    } else if(nbytes < (last->next - *endPointer)){
        struct memoryBlock *newBlock = (struct memoryBlock*)malloc(sizeof(struct memoryBlock));
        newBlock->size = nbytes;
        newBlock->isFree = 0;
        newBlock->blockAddress = last->blockAddress;
        newBlock->next = newBlock->blockAddress + nbytes;
        last->blockAddress = newBlock->next;
        allocation_count++;
        sem_post(&memorySem);
        return newBlock->blockAddress;
    }
    sem_post(&memorySem);
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
        sem_wait(&memorySem);
        struct memoryBlock *tempBlock;
        tempBlock = first;
        while(tempBlock->blockAddress != ptr){
            tempBlock = tempBlock->next;
        }
        if(tempBlock->isFree){
            printf("Block is not allocated (double free)");
        }
        else if(tempBlock != NULL){
            tempBlock->isFree = 1;
        }
        sem_post(&memorySem);
    }
}

/* get_allocated_space()
 *     Retrieve the current amount of space allocated by the memory manager (in bytes)
 *         Parameters: None
 *         Returns: int - the current number of allocated bytes
 */
int get_allocated_space()
{
    sem_wait(&memorySem);
    int sum = 0;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if(tempBlock->isFree == 0){
            sum = sum + tempBlock->size;
        }
        tempBlock = tempBlock->next;
    }
    sem_post(&memorySem);
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
    sem_wait(&memorySem);
    int sum = 0;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if(tempBlock->isFree == 1){
            sum = sum + tempBlock->size;
        }
        tempBlock = tempBlock->next;
    }
    sem_post(&memorySem);
    return sum;
}

/* get_fragment_count()
 *     Retrieve the current amount of free blocks (i.e. the count of all the block, not the size)
 *         Parameters: None
 *         Returns: int - the current number of free blocks
 */
int get_fragment_count()
{
    sem_wait(&memorySem);
    int sum = 0;
    struct memoryBlock *tempBlock;
    tempBlock = first;
    while(tempBlock != NULL){
        if(tempBlock->isFree == 1){
            sum = sum + 1;
        }
        tempBlock = tempBlock->next;
    }
    sem_post(&memorySem);
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
