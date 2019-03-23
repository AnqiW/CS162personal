/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines.
 */

#include <stddef.h>
#include "mm_alloc.h"
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

static struct metadata *head_metadata = NULL;
struct metadata{
  struct metadata *prev;
  struct metadata *next;
  int free;
  int size;
}metadata;

void *mm_malloc(size_t size) {
  //fprintf(stderr, "I'm here in mm_mallloc\n");
  //Return NULL if the requested size is 0.
  if (size == (size_t) 0 ){
    //printf("I'm returning here!");
    //fprintf(stderr, "size 0 NUll");
    return NULL;
  }

  // Get the break bound of the heap
  struct metadata *curr_meta;
  //fprintf(stderr, "before sbrk\n");
  curr_meta = (struct metadata *)sbrk(0);
  // Get the hard limit of the heap
  if(curr_meta== (void*)-1){
    return NULL;
  }


  if(head_metadata == NULL){
    // Set the base to the current place
    //fprintf(stderr, "in head_metadata == NULL condition \n ");
    head_metadata = curr_meta;
  // The heap is empty in this case
  //check whether the size of meta+ size of mem will surpass hard limit
    struct rlimit rlim;
    //fprintf(stderr, "after struct\n");
    if(getrlimit(RLIMIT_AS, &rlim) < 0){
      //fprintf(stderr, "getrlimit(RLIMIT_AS, &rlim) < 0");
      return NULL;
    }
    if (rlim.rlim_cur <  sizeof(struct metadata)+ size){
      //fprintf(stderr,"%d", rlim.rlim_cur);
      //fprintf(stderr, "size pass hard limit return Null");
      return NULL;
    }
    //fprintf(stderr, "before sbrk\n");
    //set break to contain the entire new mem
    sbrk(size+ sizeof(struct metadata));
    // create metadata
    //struct metadata *md = head_metadata;
    head_metadata->prev = NULL;
    head_metadata->next = NULL;
    head_metadata->free = 0;
    head_metadata->size = (int)size;

    //zero fill
    memset(head_metadata + sizeof(struct metadata), 0, size);
    //fprintf(stderr, "inistialize the heap, return address\n");
    //fprintf(stderr, " head_meta's size is %d\n", head_metadata->size );
    //fprintf(stderr, "head_metadata is %d \n", (int*) head_metadata);
    return head_metadata + sizeof(struct metadata);
  } else{

    // the case when the heap is not empty

    //search for the empty block from the start of the heap which is head meta metadata
    struct metadata *index_meta;
    index_meta = head_metadata;
    //fprintf(stderr, "Before iteration \n");
    //fprintf(stderr, " head_meta's size is%d\n", head_metadata->size );
    //fprintf(stderr, "index_meta = heda_meta which is %d", (int*)index_meta);

    while(index_meta!= NULL){
      //fprintf(stderr, " size requesting is%d\n",size );
      //fprintf(stderr, " index_meta's size is%d\n", index_meta->size );
      //fprintf(stderr, " index_meta's is_free is%d\n", index_meta-> free );
      if (index_meta->size >= size && index_meta-> free == 1){
        // check whether we need to splict the block
        if(index_meta->size-size-sizeof(metadata)>0){
          //need to split
          //fprintf(stderr, "need to split the block! \n");
          struct metadata *new_meta = index_meta +sizeof(struct metadata)+size;
          new_meta-> free = 1;
          new_meta-> prev = index_meta;
          new_meta-> next = index_meta->next;
          new_meta-> size = index_meta->size-size-sizeof(struct metadata);
          //update index meta
          index_meta-> next = new_meta;
          index_meta-> size = size;
          //fprintf(stderr, "splited the block! \n");
        }


        //update index-meta and return
        index_meta -> free = 0;

        memset(index_meta + sizeof(struct metadata), 0, index_meta ->size);

        //fprintf(stderr, "return address found sufficient space \n");
        return index_meta+sizeof(metadata);
      }
      index_meta = index_meta->next;
    }
    //fprintf(stderr, "I'm here line 114\n" );
    //if we are here we didn't find a sufficient space to put the a;locted memory
    //check hard_limit first
    curr_meta = sbrk(0);
    struct rlimit rlim;
    getrlimit(RLIMIT_AS,&rlim);
    if ( rlim.rlim_cur <  (int)(curr_meta-head_metadata) + sizeof(struct metadata)+ size){
      //fprintf(stderr, "already iterate through the heap, not space found, cannot expend, return Null");
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
    //fprintf(stderr, "already iterate through the heap, not space found, expend, return addr");

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
  //fprintf(stderr, "I'm here in Free\n");
    if (ptr == NULL){
      return ;
    }
    struct metadata * need_to_free = (struct metadata *) ptr-sizeof(struct metadata);
    //fprintf(stderr, "set need_to_free->free to free\n");
    need_to_free->free = 1;
    // check whether need to colasce
    //fprintf(stderr, "Before condition 1\n");
    if (need_to_free->prev != NULL && need_to_free->next !=NULL){
      //fprintf(stderr, "In condition 1\n");
      if (need_to_free->prev->free == 1 && need_to_free->next->free == 1){
        need_to_free->prev->next = need_to_free->next->next;
        need_to_free->next->next->prev = need_to_free->prev;
        need_to_free->prev->size += need_to_free->next->size+need_to_free->size;
        return;
      }
    }
    //fprintf(stderr, "Before condition 2\n");
    if (need_to_free->next != NULL){
      //fprintf(stderr, "In condition 2\n");
      if(need_to_free->next->free == 1){
        need_to_free->next->next->prev = need_to_free;
        need_to_free->next = need_to_free->next->next;
        need_to_free->size += need_to_free->next->size;
        return;
      }
    }
    //fprintf(stderr, "Before condition 3\n");
    if (need_to_free->prev != NULL){
      //fprintf(stderr, "In condition 3 \n");
      if(need_to_free->prev->free == 1){
        need_to_free->prev->next = need_to_free->next;
        need_to_free->next->prev = need_to_free->prev;
        need_to_free->prev->size += need_to_free->size;
        return;
      }
    }
    //fprintf(stderr, "After conditions\n");
}
