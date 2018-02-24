# libVT

A user-space virtual time interposition library. Wraps standard ```glibc```
calls that rely on time (```clock_gettime```, ```gettimeofday```, ```sleep```,
```usleep```, ```alarm```, ```select```, ```poll```, and ```setitimer```), and
modifies their result by a hard-coded time-dilation factor ```tdf```. e.g., if
```tdf``` is 20 and ```sleep(10)``` is called, the system will actually sleep
for 200 seconds.
