#include <stdio.h>
#include<string.h>
#include <sys/mman.h>
#include <string.h>

#define PAGE_SIZE 4096
#define NUM_PAGES 256

//Linked list structure to keep track of the blocks
typedef struct{
    size_t size;
    struct block* next;
    int free;
}block;

//Global pointer to the start of the list
static void* list_start = NULL;

void* Initialize_list(){
    //Initialize the list with the first block 
    list_start = mmap(NULL, PAGE_SIZE*NUM_PAGES, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (list_start == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    block* first_block = (block*)list_start;
    first_block->size = PAGE_SIZE*NUM_PAGES - sizeof(block);
    first_block->next = NULL;
    first_block->free = 1;
    return list_start;
}

// Function to allocate memory using mmap
void* my_malloc(size_t size) {
    // Your implementation of my_malloc goes here
    if(size == 0) return NULL;
    
    if (list_start == NULL){
        list_start = Initialize_list();
    }
    block* current = (block*)list_start;
    while (current != NULL){
        if (current->size >= size && current->free == 1){
            //Split the block
            if (current->size > size + sizeof(block)){
                block* new_block = (block*)((void*)current + sizeof(block) + size);
                new_block->size = current->size - size - sizeof(block);
                new_block->next = current->next;
                new_block->free = 1;
                current->size = size;
                current->next = new_block;
                current->free = 0;
            }
            //need to remove current from free list now!
            current->free = 0;
            return (void*)current + sizeof(block);            
        }
        if(current->next == NULL) break;
        current = current->next;
        
    }
        //printf(" current size : %d & Size asked is =%d\n", current->size,size);
        block* new_block = (block*)mmap(NULL, PAGE_SIZE*NUM_PAGES, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (new_block == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }
        new_block->size = PAGE_SIZE*NUM_PAGES - sizeof(block);
        new_block->next = NULL;
        new_block->free = 1;
        current->next = new_block;
        
        //printf("Size value : %d\n", size);
        return my_malloc(size); // can be optimized here
}

// Function to allocate and initialize memory to zero using mmap
void* my_calloc(size_t nelem, size_t size) {
    // Your implementation of my_calloc goes here
    if(nelem == 0 || size == 0) return NULL;
    
    size_t c_size = nelem*size;

    if(c_size/nelem != size) return NULL; //overflow

    void* ptr = my_malloc(c_size);
    //now initialize all values to zero

    if(ptr != NULL){
        memset(ptr, 0, c_size);
    }

    return ptr;   
}

// Function to release memory allocated using my_malloc and my_calloc
void my_free(void* ptr) {
    // Your implementation of my_free goes here
    if(ptr == NULL) return;

    if(list_start == NULL) return;//error

    block* current = (block*)list_start;
    block* previous = NULL;
    block* next_block = current->next;
    while (current != NULL){
        //printf("current = %p, ptr = %p, current->free = %d\n", current, ptr, current->free);
        if (((void*)current + sizeof(block)) == ptr && current->free == 0){
            current->free = 1;
            if(previous != NULL && previous->free == 1){
                //coalesce
                previous->size = previous->size + current->size + sizeof(block);
                previous->next = current->next;
                current = previous;
            }
             if(next_block != NULL && next_block->free == 1){
                //coalesce
                current->size = next_block->size + current->size + sizeof(block);
                current->next = next_block->next;
            }
            //if current.previous.free = 1 then coalesce
            //if current .next.free = 1 then coalesce
            return;
        }
        previous = current;
        current = current->next;        
        next_block = current->next;
    }
}

// Function to print the blocks in the free list
void info(){
    block* current = (block*)list_start;
    int i = 0;
    printf("NOTE : 0 means Not Free(Occupied) and 1 means Free\n");
    printf("Memory blocks :\n");
    while (current != NULL){
        printf("Block %d: %p, size = %d, free = %d\n", i, current, current->size, current->free);
        current = current->next;
        i++;
    }
    printf("----------------------------------------------\n");
}



