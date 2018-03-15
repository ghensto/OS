#include <stdio.h>
#include <stdlib.h>
#include "mm.h"

int main() {
  struct timeval time_s, time_e;
  void* memo_m[NUM_CHUNKS];
  gettimeofday(&time_s, NULL);
  int i = 0;
  for (i = 0; i < NUM_CHUNKS; i++) {
    memo_m[i] = malloc(CHUNK_SIZE);
  }
  for (i = 0; i < NUM_CHUNKS; i++) {
    free(memo_m[i]);
  }

  gettimeofday(&time_e, NULL);

  fprintf(stderr, "Time taken = %f msec\n",
          comp_time(time_s, time_e) / 1000.0);
  return 0;
}

// double comp_time(struct timeval time_s, struct timeval time_e) {

//   double elap = 0.0;

//   if (time_e.tv_sec > time_s.tv_sec) {
//     elap += (time_e.tv_sec - time_s.tv_sec - 1) * 1000000.0;
//     elap += time_e.tv_usec + (1000000 - time_s.tv_usec);
//   }
//   else {
//     elap = time_e.tv_usec - time_s.tv_usec;
//   }
//   return elap;
// }
