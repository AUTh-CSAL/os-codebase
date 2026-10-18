// pingpong: a parent and its child pass a number back and forth
// through two pipes.
//
//   usage: pingpong NUMBER
//
// The parent sends NUMBER to the child. The child prints it, adds 1, and
// sends the result back. The parent prints what it got back.
//
//   $ pingpong 41
//   4: received ping 41
//   3: received pong 42
//
// Lab 2: complete the four TODO lines. Each one is a single call to
// read() or write().

#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(2, "usage: pingpong NUMBER\n");
    exit(1);
  }
  int n = atoi(argv[1]);
  if (n <= 0) {
    fprintf(2, "pingpong: NUMBER must be a positive number\n");
    exit(1);
  }

  // A pipe is a one-way channel with two ends: write into p[1], read
  // from p[0]. We need one pipe for each direction.
  int p2c[2]; // parent to child
  int c2p[2]; // child to parent
  if (pipe(p2c) < 0 || pipe(c2p) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // ---- child ----
    close(p2c[1]); // the child only reads from p2c
    close(c2p[0]); // and only writes to c2p
    int x = 0;

    // TODO 1: read the number from the parent into x.

    printf("%d: received ping %d\n", getpid(), x);
    x = x + 1;

    // TODO 2: write x back to the parent.

    exit(0);
  }

  // ---- parent ----
  close(p2c[0]); // the parent only writes to p2c
  close(c2p[1]); // and only reads from c2p
  int y = 0;

  // TODO 3: write n to the child.

  // TODO 4: read the child's answer into y.

  wait(0);
  printf("%d: received pong %d\n", getpid(), y);
  exit(0);
}
