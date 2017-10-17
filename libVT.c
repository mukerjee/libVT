#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define SECONDS_TO_NS 1000000000

static float tdf = 20.0;
static u64 start_time = 0;

inline u64 timespec_to_ns(struct timespec *tp) {
  return tp->tv_sec * SECONDS_TO_NS + tp->tv_nsec;
}

inline void ns_to_timespec(u64 ns, struct timespec *tp) {
  tp->tv_sec = ns / SECONDS_TO_NS;
  tp->tv_nsec = ns % SECONDS_TO_NS;
}

// Find the library version of the function that we are wrapping
static void get_next_fn(void** next_fn, char* fn) {
  char* msg;

  if(! *next_fn){
    *next_fn = dlsym(RTLD_NEXT, fn);
    if ((msg = dlerror()) != NULL) {
      fprintf(stderr, "dlopen failed on %s: %s\n", fn, msg);
      exit(1);
    } else {
      /* fprintf(stderr,  "next_%s = %p\n", fn, *next_fn); */
    }
  }
}

// Wrapped clock_gettime()
// Assumes the app only uses one clk_id... should eventually redo to handle multiple.
int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  static int (*next_clock_gettime)(clockid_t clk_id, struct timespec *tp) = NULL;
  char* fn_name = "clock_gettime";
  get_next_fn((void**)&next_clock_gettime,fn_name);

  int rc = next_clock_gettime(clk_id, tp);
  
  if (!start_time)
    start_time = timespec_to_ns(tp);

  u64 now = timespec_to_ns(tp);
  u64 dilated_now = start_time + (now - start_time) / tdf;

  ns_to_timespec(dilated_now, tp);
  
  return rc;
}
