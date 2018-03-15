#include <stdio.h>
#include "mm.h"
int main() {
  struct timeval time_s, time_e;

  /* start timer */
  gettimeofday (&time_s, NULL);

  /* TODO - code you wish to time goes here */
  mm_t memo_m;
  mm_init(&memo_m, NUM_CHUNKS, CHUNK_SIZE);
  int i;
  for (i = 0; i < NUM_CHUNKS; i++) {
    mm_get(&memo_m);
  }
  for (i = 0; i < NUM_CHUNKS; i++) {
    mm_put(&memo_m, memo_m.memory_ptr + i*memo_m.chunk_size);
  }
  mm_release(&memo_m);
  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);

  // Test edge cases (trying to get more chunks than allocated)
  mm_t test_mm;
  mm_init(&test_mm, NUM_CHUNKS, 50);
  for (int i = 0; i < NUM_CHUNKS + 10; i++) {
      if (mm_get(&test_mm) == NULL) {
          printf("NULL returned at index %d\n", i);
      }
  }
  mm_release(&test_mm);

  return 0;
}
