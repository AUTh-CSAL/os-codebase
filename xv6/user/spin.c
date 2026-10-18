// spin: compute forever, without ever making a system call.
// Only an interrupt can take the processor away from it.

#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  volatile uint64 n = 0;
  for (;;)
    n++;
}
