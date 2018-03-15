#include <stdio.h>
#include <stdlib.h>

#include "mm.h"

/* Return usec */
double comp_time(struct timeval time_s, struct timeval time_e) {

  double elap = 0.0;

  if (time_e.tv_sec > time_s.tv_sec) {
    elap += (time_e.tv_sec - time_s.tv_sec - 1) * 1000000.0;
    elap += time_e.tv_usec + (1000000 - time_s.tv_usec);
  }
  else {
    elap = time_e.tv_usec - time_s.tv_usec;
  }
  return elap;
}

/* TODO - Implement.  Return 0 for success, or -1 and set errno on fail. */
int mm_init(mm_t *mm, int hm, int sz) {
  // set attributes of mm (including creating and filling the stack)
  mm->chunk_size = sz;
  mm->chunk_num = hm;

  // malloc memory
  if ((mm->memory_ptr = malloc(hm*sz)) == NULL) {
      perror("Failed to allocate memory.\n");
      return -1;
  }

  // Allocate memory for an array of pointers, each pointer will later be initialized to point to a memory chunk
  // This array will work like a stack, using the member variable stack_top to keep track of the stack
  if ((mm->open_memory = (void**) malloc(sizeof(void*) * hm)) == NULL) {
      perror("Failed to allocate memory.\n");
      return -1;
  }

  // Now assign the stack top to the index of the first pointer to the memory chunk
  mm->stack_top = 0;

  //Initialize the array of pointers
  int index;
  for(index=0; index < hm; index++) {
     mm->open_memory[index] = mm->memory_ptr + index * sz;
  }
  // The whole structure will look like this
  /* |chunk|chunk|chunk|chunk|chunk|chunk|chunk|chunk|
        |     |     |     |     |     |     |     |
     |  F  |  F  |  F  |  F  |  F  |  F  |  F  |  F  |
      stack_top
      */
  return 0;
}

void *mm_get(mm_t *mm) {
  // Each time this function is called, assign a pointer to next available memory chunk to return
  // Increment stack top (consider this as 'popping' that pointer out of the stack)
  // so that the stack only has pointers to free memory chunk
  // |  A  |  A  |  A  |  F  |  F  |  F  |  F  |  F  | (before)
  //                    stack_top
  //
  // |  A  |  A  |  A  |  A  |  F  |  F  |  F  |  F  | (after)
  //                          stack_top

  /* Check if stack is empty */
  if (mm->stack_top < mm->chunk_num) {
      void* chunk = mm->open_memory[mm->stack_top];
      mm->stack_top++;
      return chunk;
  } else {
      return NULL;
  }
}

void mm_put(mm_t *mm, void *chunk) {
  // Each time this function is called, decrement stack top (consider this as 'pushing' the pointer chunk
  // into the stack).
  // |  A  |  A  |  A  |  A  |  F  |  F  |  F  |  F  | (before)
  //                          stack_top
  //
  // |  A  |  A  |  A  |  F  |  F  |  F  |  F  |  F  | (after)
  //                    stack_top
  mm->stack_top--;
  mm->open_memory[mm->stack_top] = chunk;
  chunk = 0;
}

/* TODO - Implement */
void mm_release(mm_t *mm) {
  // call free
  free(mm->memory_ptr);
  free(mm->open_memory);
}

/*
 * TODO - This is just an example of how to use the timer.  Notice that
 * this function is not included in mm_public.h, and it is defined as static,
 * so you cannot call it from other files.  Instead, just follow this model
 * and implement your own timing code where you need it.
 */
static void timer_example() {
  struct timeval time_s, time_e;

  /* start timer */
  gettimeofday (&time_s, NULL);

  /* TODO - code you wish to time goes here */

  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);
}
