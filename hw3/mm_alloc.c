/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines.
 */

#include <stddef.h>
#include "mm_alloc.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

struct metadata *head_metadata = NULL;
struct metadata{
  struct metadata *prev;
  struct metadata *next;
  int free;
  int size;
}metadata;

void *mm_malloc(size_t size) {
  //Return NULL if the requested size is 0.
  if (size == (size_t) 0 ){
    //printf("I'm returning here!");
    return NULL;
  }
  // Get the break bound of the heap
  struct metadata *curr_meta;
  curr_meta = (struct metadata *)sbrk(0);
  // Get the hard limit of the heap
  if(curr_meta== (void*)-1){
    return NULL;
  }

  if(head_metadata == NULL){
    //-----------------------------------------WHen the heap is empty --------------------
    // Set the base to the current place
    head_metadata = curr_meta;
  // The heap is empty in this case
  //check whether the size of meta+ size of mem will surpass hard limit
    struct rlimit rlim;
    if (getrlimit(RLIMIT_AS,&rlim) <  (int)sizeof(struct metadata)+ size){
      return NULL;
    }
    //set break to contain the entire new mem
    void * addr = sbrk(size+ sizeof(struct metadata));
    // create metadata
    struct metadata *md = head_metadata;
    md->prev = NULL;
    md->next = NULL;
    md->free = 0;
    md->size = size;

    //zero fill
    memset(addr + sizeof(struct metadata), 0, size);
    return addr + sizeof(struct metadata);
    //---------------------------------------------When the heap is empty-----------------------=
  }


  else{

    // the case when the heap is not empty

    //search for the empty block from the start of the heap which is head meta metadata
    struct metadata *index_meta;
    index_meta = head_metadata;
    while( index_meta->next != NULL ){
      if (index_meta->size > size && index_meta-> free == 1){
        // check whether we need to splict the block
        if(index_meta->size-size-2*sizeof(metadata)>0){
          //need to split
          struct metadata *new_meta = index_meta +sizeof(struct metadata)+size;
          new_meta-> free = 1;
          new_meta-> prev = index_meta;
          new_meta-> next = index_meta->next;
          new_meta-> size = index_meta->size-size-sizeof(struct metadata);
          //update index meta
          index_meta-> next = new_meta;
          index_meta-> size = size;
        }

        //update index-meta and return
        index_meta -> free = 0;
        //check whether we need to pad the allocted memory.
        /*
        if (index_meta-> size >size){
          //do memset?
          memset(index_meta+sizeof(metadata)+size, 0, index_meta -> size-size);
        }*/

        memset(index_meta + sizeof(struct metadata), 0, index_meta ->size);
        return index_meta+sizeof(metadata);
      }
      index_meta = index_meta->next;
    }
    //if we are here we didn't find a sufficient space to put the a;locted memory
    //check hard_limit first
    curr_meta = sbrk(0);
    struct rlimit rlim;
    if (getrlimit(RLIMIT_AS,&rlim) <  (int)(curr_meta-head_metadata) + sizeof(struct metadata)+ size){
      return NULL;
    }
    //use sbrk to creae more space on the heap
    sbrk(size+sizeof(metadata));
    struct metadata *md = curr_meta;
    md->prev = index_meta;
    md->next = NULL;
    md->free = 0;
    md->size = size;

    memset(curr_meta + sizeof(struct metadata), 0, size);

    return curr_meta + sizeof(struct metadata);

  }




  }

    /* YOUR CODE HERE */
    // First Fit, Start from the bottom(start_of_heap)of the heap and search up.





void *mm_realloc(void *ptr, size_t size) {
    /* YOUR CODE HERE */
    return NULL;
}

void mm_free(void *ptr) {
    /* YOUR CODE HERE */
}
