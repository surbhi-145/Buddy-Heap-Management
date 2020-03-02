#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>

#define MAXMEM 1024
#define MAXINDEX 11
#define ALLOCLENGTH 70
#define HEADERSIZE 8

typedef struct freeList
{
    unsigned int size;
    char *start_address;
    struct freeList *next;

} freeList;

typedef struct allocList
{
    char name[40];
    int size;
    char *start_address;
    struct allocList *next;

} allocList;

char *baseEndAddress;
freeList *myFree[MAXINDEX];
allocList alloc[ALLOCLENGTH];
int alloc_index = 0;

void updateAllocIndex()
{
    for (int i = 0; i < ALLOCLENGTH; i += 1)
    {
        if (alloc[i].start_address == 0)
        {
            alloc_index = i;
            break;
        }
    }
}

void initialize()
{
    myFree[MAXINDEX - 1] = malloc(MAXMEM);
    myFree[MAXINDEX - 1]->next = NULL;
    myFree[MAXINDEX - 1]->size = MAXMEM;
    myFree[MAXINDEX - 1]->start_address = (char *)myFree[MAXINDEX - 1];
    baseEndAddress=(char*)(myFree[MAXINDEX - 1]->start_address + myFree[MAXINDEX - 1]->size);
    for (int i = 0; i < 10; i += 1)
    {
        myFree[i] = NULL;
    }

    for (int i = 0; i < ALLOCLENGTH; i++)
    {
        alloc[i].start_address = 0;
    }
}

int nextPowerOf2(int x)
{
    if (x == 1)
        return 0;

    int y = 2, pow = 1;

    while (y < x)
    {
        y = y * 2;
        pow += 1;
    }

    return y;
}

int correctSize(int size)
{
    if (size < HEADERSIZE)
    {
        return HEADERSIZE * 2;
    }

    return (nextPowerOf2(size));
}

int base(int size)
{
    int n = MAXMEM;
    int ans = MAXINDEX - 1;

    while (n > size)
    {
        n = n / 2;
        ans--;
    }

    return ans;
}

freeList *findFree(int i)
{
    freeList *result = NULL;
    int flag = 0;
    for (int j = i; i < MAXINDEX && flag == 0; j++)
    {
        if (myFree[j] != NULL)
        {
            freeList *ptr = myFree[j];
            freeList *prev;
            flag = 1;

            if (ptr->next == NULL)
            {
                result = ptr;
                myFree[j] = NULL;
            }

            else
            {
                prev = ptr;
                ptr = ptr->next;

                while (ptr->next != NULL)
                {
                    prev = ptr;
                    ptr = ptr->next;
                }

                result = ptr;
                ptr = NULL;
            }
        }
    }

    return result;
}

freeList *divideList(freeList *block, int index, int size)
{
    freeList *result = (freeList *)((block->start_address) + ((block->size) / 2));
    result->next = NULL;
    result->size = (block->size) / 2;
    result->start_address = (char *)result;

    block->next = myFree[index - 1];
    myFree[index - 1] = block;
    block->size = result->size;
    index -= 1;

    if (index != size)
    {
        result = divideList(result, index, size);
    }

    return result;
}

freeList *allocate(int size, char *name)
{
    size = correctSize(size);
    int i = base(size);
    freeList *block = findFree(i);
    int index = base(block->size);
    if (index != i)
    {
        block = divideList(block, index, i);
    }

    updateAllocIndex();
    allocList *new = alloc + alloc_index;
    new->size = size;
    new->start_address = (char *)block;
    strcpy(new->name, name);
    return block;
}

freeList *deleteNode(freeList *prev, freeList *ptr, int index)
{
    freeList *head = myFree[index];
    if (prev != NULL)
    {
        prev->next = ptr->next;
    }

    else
    {
        head = NULL;
    }

    return head;
}

freeList *join(freeList *block, int index)
{
    freeList *result = block;
    freeList *prev = NULL;
    freeList *ptr = myFree[index];
    int flag = 0;
    if (ptr == NULL && index < MAXINDEX)
    {
        result = block;
    }

    else
    {
        char* buddyAddress;
        int buddyNum=(int)(baseEndAddress-block->start_address-block->size);
        buddyNum=buddyNum/block->size;
        if(buddyNum%2==0)
        {
            buddyAddress=(char*)(block->start_address - block->size);
        }
        else
        {
            buddyAddress=(char*)(block->start_address + block->size);
        }       
        while (ptr != NULL && flag == 0)
        {
            if (ptr->start_address == buddyAddress)
            {
                flag = 1;
                myFree[index] = deleteNode(prev, ptr, index);
                ptr->size *= 2;
                block->size *=2;
                if (index < MAXINDEX)
                {
                    if(buddyNum%2==0)
                        result = join(ptr, index + 1);
                    else
                        result = join(block, index + 1);
                }
            }
            else
            {
                prev = ptr;
                ptr = ptr->next;
            }
        }
    }
    return result;
}

void freeBlock(char *name)
{
    int flag = 0;

    for (int i = 0; i < ALLOCLENGTH && flag == 0; i++)
    {
        if (alloc[i].start_address != 0 && strcmp((alloc + i)->name, name) == 0)
        {
            flag = 1;
            freeList *block = (freeList *)(alloc[i].start_address);
            alloc[i].start_address = 0;
            block->size = alloc[i].size;
            block->start_address = (char *)block;
            block->next = NULL;
            int index = base(block->size);
            block = join(block, index);
            index = base(block->size);
            block->next = myFree[index];
            myFree[index] = block;
        }
    }
}

void printMemory()
{
    printf("--------------------------------------\n");
    printf("Free List : \n");
    for (int i = 0; i < MAXINDEX; i++)
    {
        printf("%d: ", i);
        freeList *ptr = myFree[i];
        if (ptr == NULL)
        {
            printf("Empty\n");
        }

        else
        {
            while (ptr != NULL)
            {
                printf("Address - (%p,%p)", ptr->start_address, ((ptr->start_address) + (ptr->size) - 1));
                printf("Size- %d\n", ptr->size);
                ptr = ptr->next;
            }
        }
    }

    printf("--------------------------------------\n");
    printf("Alloc List: \n");
    int count = 0;
    for (int i = 0; i < ALLOCLENGTH; i++)
    {
        if (alloc[i].start_address != 0)
        {
            count += 1;
            printf("Address: (%p,%p) ", alloc[i].start_address, ((alloc[i].start_address) + (alloc[i].size) - 1));
            printf("Size: %d\n", alloc[i].size);
        }
    }

    if (count == 0)
        printf("Empty.");

    printf("\n");

    printf("--------------------------------------\n");
}

int main()
{

    initialize();
    printMemory();
    freeList *new1= allocate(32, "new1");
    printMemory();
    freeList *new2 = allocate(32, "new2");
    printMemory();
    freeList *new3 = allocate(32, "new3");
    printMemory();
    freeList *new4 = allocate(32, "new4");
    printMemory();
    freeBlock("new1");
    printMemory();
    freeBlock("new3");
    printMemory();
    freeBlock("new2");
    printMemory();
    freeBlock("new4");
    printMemory();
    return 0;
}