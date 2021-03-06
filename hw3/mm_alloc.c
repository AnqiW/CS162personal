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


struct metadata{
  struct metadata *prev;
  struct metadata *next;
  int free;
  int size;
}metadata;
static struct metadata *head_metadata = NULL;

void *mm_malloc(size_t size) {
  fprintf(stderr, "I'm here in mm_mallloc\n");
  //Return NULL if the requested size is 0.
  if (size == (size_t) 0 ){
    //printf("I'm returning here!");
    fprintf(stderr, "size 0 NUll");
    return NULL;
  }

  // Get the break bound of the heap
  struct metadata *curr_meta;
  fprintf(stderr, "before sbrk\n");
  curr_meta = (struct metadata *)sbrk(0);
  fprintf(stderr, "after sbrk\n");
  // Get the hard limit of the heap
  if(curr_meta== (void*)-1){
    return NULL;
  }


  if(head_metadata == NULL){
    // Set the base to the current place
    fprintf(stderr, "in head_metadata == NULL condition \n ");
    head_metadata = curr_meta;
  // The heap is empty in this case
  //check whether the size of meta+ size of mem will surpass hard limit
    struct rlimit rlim;
    fprintf(stderr, "after struct\n");
    if(getrlimit(RLIMIT_AS, &rlim) < 0){
      fprintf(stderr, "getrlimit(RLIMIT_AS, &rlim) < 0");
      return NULL;
    }
    if (rlim.rlim_cur <  sizeof(struct metadata)+ size){
      fprintf(stderr,"%d", rlim.rlim_cur);
      fprintf(stderr, "size pass hard limit return Null");
      return NULL;
    }
    fprintf(stderr, "before sbrk\n");
    //set break to contain the entire new mem
    if(sbrk((int)size+ (int)sizeof(struct metadata))== (void*)-1){
      return NULL;
    }
    // create metadata
    //struct metadata *md = head_metadata;
    head_metadata->prev = NULL;
    head_metadata->next = NULL;
    head_metadata->free = 0;
    head_metadata->size = (int)size;

    //zero fill
    memset(head_metadata + sizeof(struct metadata), 0, size);
    fprintf(stderr, "inistialize the heap, return address\n");
    fprintf(stderr, " head_metadata + sizeof(struct metadata) is %d\n", (int)head_metadata + (int)sizeof(struct metadata) );
    fprintf(stderr, "head_metadata is %d \n", (int*) head_metadata);
    return (int)head_metadata + (int)sizeof(struct metadata);
  } else{

    // the case when the heap is not empty

    //search for the empty block from the start of the heap which is head meta metadata
    struct metadata *index_meta;
    index_meta = head_metadata;
    struct metadata *last_meta;
    fprintf(stderr, "Before iteration \n");
    fprintf(stderr, " head_meta's size is%d\n", head_metadata->size );
    fprintf(stderr, "index_meta = heda_meta which is %d", (int*)index_meta);

    while(index_meta!= NULL){
      fprintf(stderr, " size requesting is%d\n",size );
      fprintf(stderr, " index_meta's size is%d\n", index_meta->size );
      fprintf(stderr, " index_meta's is_free is%d\n", index_meta-> free );

      if (index_meta->size >= size && index_meta-> free == 1){
        // check whether we need to splict the block-------------------------block splitting______________________________
        if((int)index_meta->size-(int)size-(int)sizeof(metadata)>0){

          fprintf(stderr, "need to split the block! \n");

          *(struct metadata *) ((int)index_meta +(int)sizeof(struct metadata)+(int)size)=(struct metadata){.free=1,.prev=index_meta,.next=index_meta->next,
            .size=(int)index_meta->size-size-sizeof(struct metadata)};

          fprintf(stderr, "after initilizing" );
          index_meta-> next = (struct metadata *) ((int)index_meta +(int)sizeof(struct metadata)+(int)size);
          index_meta-> size = size;
          fprintf(stderr, "splited the block! \n");
        }
        //----------------------------------block spliting__________________________________________________-


        //update index-meta and return
        index_meta -> free = 0;

        memset((int)index_meta + (int)sizeof(struct metadata), 0, index_meta ->size);

        fprintf(stderr, "return address found sufficient space \n");
        return (int)index_meta+(int)sizeof(metadata);
      }
      last_meta = index_meta;
      index_meta = index_meta->next;
    }
    fprintf(stderr, "I'm here line 114\n" );
    //if we are here we didn't find a sufficient space to put the a;locted memory
    //check hard_limit first
    curr_meta = sbrk(0);

    if(curr_meta== (void*)-1){
      return NULL;
    }
    struct rlimit rlim;
    getrlimit(RLIMIT_AS,&rlim);
    if ( rlim.rlim_cur <  (int)(curr_meta-head_metadata) + sizeof(struct metadata)+ size){
      fprintf(stderr,"I 'm here in rlim.rlim_cur'\n" );
      //fprintf(stderr, "already iterate through the heap, not space found, cannot expend, return Null");
      return NULL;
    }
    //use sbrk to creae more space on the heap
    sbrk(size+sizeof(metadata));
    struct metadata *md = curr_meta;

    md->prev = last_meta;
    md->next = NULL;
    md->free = 0;
    md->size = size;
    last_meta->next = md;

    memset((int)curr_meta + (int)sizeof(struct metadata), 0, size);
    fprintf(stderr, "already iterate through the heap, not space found, expend, return addr\n");
    fprintf(stderr,"here\n" );
    fprintf(stderr, "md->prev= %d\n",(int*) md->prev );

    return (int)curr_meta + (int)sizeof(struct metadata);

  }



  }
    /* YOUR CODE HERE */
    // First Fit, Start from the bottom(start_of_heap)of the heap and search up.
void *mm_realloc(void *ptr, size_t size) {
    /* YOUR CODE HERE */
    if (size==0 && ptr!= NULL){
      mm_free(ptr);
      return NULL;
    }
    if (ptr==NULL){
      return mm_malloc(size);
    }
    struct metadata * need_to_resize = (int)ptr-(int)sizeof(struct metadata);
    //if new size is smaller than the original size, resize the original one.
    if ((int)size == need_to_resize->size){
      return ptr;
    }
    if ((int)size < need_to_resize->size){
      //create a new block of smaller size s and only copy over the first s bytes

      mm_free(ptr);
      struct metadata * new_place = mm_malloc(size);
      if(new_place ==NULL){
        return NULL;
      }
      memcpy(new_place, ptr, size);
      return new_place ;

    }

    //if cannot allocate the new requested size, do not modity the original block
    if (mm_malloc(size)==NULL){
      return NULL;
    }
    //if larger, free and mm_alloc and then memcpy
    struct metadata * new_place = mm_malloc(size);
    if(new_place ==NULL){
      return NULL;
    }
    memcpy(new_place, ptr, need_to_resize->size);
    mm_free(ptr);
    return new_place;

}

void mm_free(void *ptr) {
  //fprintf(stderr, "I'm here in Free\n");
    if (ptr == NULL){
      return ;
    }

    struct metadata * need_to_free = (int)ptr-(int)sizeof(struct metadata);
    fprintf(stderr, "need_to_free is %d\n", need_to_free);
    fprintf(stderr, "size of metadata is %d\n", sizeof(struct metadata));
    fprintf(stderr, "set need_to_free->free to free\n");
    need_to_free->free = 1;
    fprintf(stderr, "need_to_free->free is %d\n",need_to_free->free );
    fprintf(stderr, "need_to_free->next is %d\n",need_to_free->next );
    // check whether need to colasce
    fprintf(stderr, "Before condition 1\n");
    if (need_to_free->prev != NULL && need_to_free->next !=NULL){
      fprintf(stderr, "In condition 1\n");
      if (need_to_free->prev->free == 1 && need_to_free->next->free == 1){
        need_to_free->prev->size += need_to_free->next->size+need_to_free->size;
        need_to_free->prev->next = need_to_free->next->next;
        need_to_free->next->next->prev = need_to_free->prev;

        return;
      }
    }
    fprintf(stderr, "Before condition 2\n");
    if (need_to_free->next != NULL){
      fprintf(stderr, "In condition 2\n");
      fprintf(stderr, "need_to_free->free is %d\n",need_to_free->free);
      if(need_to_free->next->free == 1){
        need_to_free->size += need_to_free->next->size;
        if (need_to_free->next->next != NULL){
          need_to_free->next->next->prev = need_to_free;
        }
        fprintf(stderr, "Before next\n");
        need_to_free->next = need_to_free->next->next;


        fprintf(stderr, "After size");
        return;
      }
    }
    fprintf(stderr, "Before condition 3\n");
    fprintf(stderr, "need_to_free->prev is %d\n",need_to_free->prev );
    if (need_to_free->prev != NULL){
      fprintf(stderr, "In condition 3 \n");
      fprintf(stderr, "need_to_free->free is %d\n",need_to_free->free);
      fprintf(stderr, "need_to_free->next is %d\n",need_to_free->next);
      fprintf(stderr, "need_to_free->prev->free  is %d\n",need_to_free->prev->free );
      if(need_to_free->prev->free == 1){
        need_to_free->prev->size += need_to_free->size;
        if (need_to_free->prev->prev != NULL){
          need_to_free->prev->prev->next = need_to_free->next;
        }
        if (need_to_free->next!=NULL){
          need_to_free->next->prev = need_to_free->prev;
        }

        fprintf(stderr, "need_to_free->prev->size  is %d\n",need_to_free->prev->size );
        return;
      }
    }
    return;
}
