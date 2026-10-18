// peek: try to do, from a user program, what only the kernel may do.
//
//   peek        read the kernel's memory
//   peek csr    execute an instruction reserved for supervisor mode

#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "csr") == 0) {
    // Read the supervisor status register: allowed only in supervisor mode.
    uint64 x;
    printf("peek: reading register sstatus...\n");
    asm volatile("csrr %0, sstatus" : "=r"(x));
    printf("peek: sstatus is 0x%lx\n", x); // never printed
    exit(0);
  }

  // The kernel's code starts here (KERNBASE in kernel/memlayout.h).
  uint64 addr = 0x80000000;
  printf("peek: reading the kernel's memory at 0x%lx...\n", addr);
  int value = *(volatile int *)addr;
  printf("peek: it holds %d\n", value); // never printed
  exit(0);
}
