#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define TDF 20

int clock_gettime(clockid_t clk_id, struct timespec *tp);
static int (*next_clock_gettime)(clockid_t clk_id, struct timespec *tp) = NULL;

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
int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  char* fn_name = "clock_gettime";
  get_next_fn((void**)&next_clock_gettime,fn_name);
  printf("in clock_gettime");
  return next_clock_gettime(clk_id, tp);
}
