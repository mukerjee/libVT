#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <poll.h>

#define SECONDS_TO_NS 1000000000
#define US_TO_NS 1000

static unsigned int tdf = 20;

inline unsigned long long timespec_to_ns(const struct timespec *tp) {
  return tp->tv_sec * SECONDS_TO_NS + tp->tv_nsec;
}

inline unsigned long long timeval_to_ns(const struct timeval *tv) {
  return tv->tv_sec * SECONDS_TO_NS + tv->tv_usec * US_TO_NS;
}

inline void ns_to_timespec(unsigned long long ns, struct timespec *tp) {
  tp->tv_sec = ns / SECONDS_TO_NS;
  tp->tv_nsec = ns % SECONDS_TO_NS;
}

inline void ns_to_timeval(unsigned long long ns, struct timeval *tv) {
  tv->tv_sec = ns / SECONDS_TO_NS;
  tv->tv_usec = (ns % SECONDS_TO_NS) / US_TO_NS;
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
  static int (*next_clock_gettime)(clockid_t, struct timespec*) = NULL;
  get_next_fn((void**)&next_clock_gettime, "clock_gettime");

  int rc = next_clock_gettime(clk_id, tp);
  
  unsigned long long now = timespec_to_ns(tp);
  ns_to_timespec(now / tdf, tp);
  
  return rc;
}

// Wrapped gettimeofday()
int gettimeofday(struct timeval *tv, struct timezone *tz) {
  static int (*next_gettimeofday)(struct timeval*, struct timezone*) = NULL;
  get_next_fn((void**)&next_gettimeofday, "gettimeofday");

  int rc = next_gettimeofday(tv, tz);
  
  unsigned long long now = timeval_to_ns(tv);
  ns_to_timeval(now / tdf, tv);
  
  return rc;
}

// Wrapped sleep()
unsigned int sleep(unsigned int seconds) {
  static unsigned int (*next_sleep)(unsigned int);
  get_next_fn((void**)&next_sleep, "sleep");

  return next_sleep(seconds * tdf);
}

// Wrapped select()
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout) {
  static int (*next_select)(int, fd_set*, fd_set*, fd_set*, struct timeval *) = NULL;
  get_next_fn((void**)&next_select, "select");

  unsigned long long t = timeval_to_ns(timeout);
  ns_to_timeval(t * tdf, timeout);

  return next_select(nfds, readfds, writefds, exceptfds, timeout);
}

// Wrapped poll()
int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
  static int (*next_poll)(struct pollfd*, nfds_t, int);
  get_next_fn((void**)&next_poll, "poll");

  return next_poll(fds, nfds, timeout * tdf);
}

// Wrapped setitimer()
int setitimer(__itimer_which_t which, const struct itimerval *new_value,
              struct itimerval *old_value) {
  static int (*next_setitimer)(__itimer_which_t, const struct itimerval*,
                               struct itimerval*) = NULL;
  get_next_fn((void**)&next_setitimer, "setitimer");

  unsigned long long next_val = timeval_to_ns(&new_value->it_interval);
  unsigned long long curr_val = timeval_to_ns(&new_value->it_value);

  struct itimerval nv;
  ns_to_timeval(next_val * tdf, &nv.it_interval);
  ns_to_timeval(curr_val * tdf, &nv.it_value);

  return next_setitimer(which, &nv, old_value);
}
