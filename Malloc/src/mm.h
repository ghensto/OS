#ifndef __MM_H
#define __MM_H

#include <sys/time.h>

#define INTERVAL 0
#define INTERVAL_USEC 50000
#define CHUNK_SIZE 64
#define NUM_CHUNKS 1000000

/* TODO - Fill this in */
typedef struct {

	int chunk_num; // how many chunks
	int chunk_size; // chunk size
	void* memory_ptr; // pointer to start of all memory
	void** open_memory; // stack of open memory
  	int stack_top; // index at the top of the stack

} mm_t;

/* TODO - Implement these in mm.c */
double comp_time(struct timeval time_s, struct timeval time_e);
int mm_init(mm_t *mm, int num_chunks, int chunk_size);
void *mm_get(mm_t *mm);
void mm_put(mm_t *mm, void *chunk);
void mm_release(mm_t *mm);

#endif
