// proof: print a proof-of-work token for the OS lab quizzes on Moodle.
//
//   usage: proof <lab> <username> <code>
//
// Moodle shows the exact command to run. The token combines the lab number,
// your Moodle username, the attempt's code, the current time, a fingerprint
// of the computer it was built on, and evidence that your xv6 works as the
// lab requires. See tokenlib.py for the format.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Fingerprint of the installation: the Makefile passes a hash of the Linux
// machine id here when it compiles this file.
#ifndef MACHINE_FP
#error "MACHINE_FP not set: build xv6 with its own Makefile (make qemu)"
#endif

static const char CHECK_ALPHABET[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

// FNV-1a 64 followed by the splitmix64 finalizer.
static uint64
H(const char *s)
{
  uint64 h = 0xcbf29ce484222325ULL;
  for (; *s; s++) {
    h ^= (uchar)*s;
    h *= 0x100000001b3ULL;
  }
  h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9ULL;
  h = (h ^ (h >> 27)) * 0x94d049bb133111ebULL;
  return h ^ (h >> 31);
}

// Append helpers: xv6 has no snprintf.
static char *
putstr(char *p, const char *s)
{
  while (*s)
    *p++ = *s++;
  *p = 0;
  return p;
}

static char *
putdec(char *p, uint64 x)
{
  char tmp[24];
  int n = 0;
  do {
    tmp[n++] = '0' + x % 10;
    x /= 10;
  } while (x);
  while (n > 0)
    *p++ = tmp[--n];
  *p = 0;
  return p;
}

static char *
puthex4(char *p, uint x)
{
  for (int shift = 12; shift >= 0; shift -= 4)
    *p++ = "0123456789ABCDEF"[(x >> shift) & 0xF];
  *p = 0;
  return p;
}

static int
fail(const char *msg)
{
  fprintf(2, "proof: %s\n", msg);
  return -1;
}

static int
hexval(const char *s)
{
  int v = 0;
  for (; *s; s++)
    v = v * 16 + (*s <= '9' ? *s - '0' : *s - 'a' + 10);
  return v;
}

// Parse one line "<pid>: received <word> <value>\n" at *s, advancing *s.
static int
parse_line(char **s, const char *word, int *pid, int *value)
{
  char *p = *s;
  int neg = 0;

  if (*p < '0' || *p > '9')
    return -1;
  for (*pid = 0; *p >= '0' && *p <= '9'; p++)
    *pid = *pid * 10 + (*p - '0');
  for (const char *w = ": received "; *w; w++)
    if (*p++ != *w)
      return -1;
  for (const char *w = word; *w; w++)
    if (*p++ != *w)
      return -1;
  if (*p++ != ' ')
    return -1;
  if (*p == '-') {
    neg = 1;
    p++;
  }
  if (*p < '0' || *p > '9')
    return -1;
  for (*value = 0; *p >= '0' && *p <= '9'; p++)
    *value = *value * 10 + (*p - '0');
  if (neg)
    *value = -*value;
  if (*p++ != '\n')
    return -1;
  *s = p;
  return 0;
}

// Lab 1: you built and booted xv6, and it can read its file system.
static int
lab1(uint64 *e)
{
  struct stat st;

  if (stat("README", &st) < 0)
    return fail("cannot find README: is this the course's xv6?");
  *e = st.size;
  return 0;
}

// Lab 2: run your pingpong with a number derived from the code, capture
// what it prints, and check that the child got the ping, the parent got
// the pong, and the number travelled correctly both ways.
static int
lab2(const char *nonce, uint64 *e)
{
  int v = hexval(nonce) % 1000000 + 1; // 1 .. 1000000
  char arg[16], out[256];
  int fds[2], n = 0, r, status;

  putdec(arg, v);
  if (pipe(fds) < 0)
    return fail("pipe failed");
  int pid = fork();
  if (pid < 0)
    return fail("fork failed");
  if (pid == 0) {
    // Child: send standard output (fd 1) into the pipe, run pingpong.
    close(1);
    dup(fds[1]);
    close(fds[0]);
    close(fds[1]);
    char *argv[] = {"pingpong", arg, 0};
    exec("pingpong", argv);
    fprintf(2, "proof: cannot run pingpong. Did you add $U/_pingpong to "
               "UPROGS in the Makefile, and rebuild with make qemu?\n");
    exit(1);
  }
  close(fds[1]);
  while (n < sizeof(out) - 1 && (r = read(fds[0], out + n, sizeof(out) - 1 - n)) > 0)
    n += r;
  out[n] = 0;
  close(fds[0]);
  wait(&status);
  if (status != 0)
    return -1;

  printf("pingpong %s printed:\n%s\n", arg, out);
  char *s = out;
  int cpid, cval, ppid, pval;
  if (parse_line(&s, "ping", &cpid, &cval) < 0
      || parse_line(&s, "pong", &ppid, &pval) < 0 || *s)
    return fail("expected exactly two lines, \"PID: received ping N\" "
                "and then \"PID: received pong N+1\"");
  if (ppid != pid)
    return fail("the pong line must be printed by the parent");
  if (cpid == pid)
    return fail("the ping line must be printed by the child, not the parent");
  if (cval != v)
    return fail("the child did not receive the number the parent sent "
                "(check TODO 1 and TODO 3)");
  if (pval != v + 1)
    return fail("the parent did not receive the child's answer "
                "(check TODO 2 and TODO 4)");
  *e = pval;
  return 0;
}

// Lab-specific evidence: something only a working xv6 for that lab produces.
static int
evidence(int lab, const char *nonce, uint64 *e)
{
  switch (lab) {
  case 1:
    return lab1(e);
  case 2:
    return lab2(nonce, e);
  default:
    return fail("this xv6 has no proof for that lab yet: "
                "get course updates with git pull upstream main");
  }
}

static int
valid_nonce(const char *s)
{
  int n = 0;
  for (; *s; s++, n++)
    if (!((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f')))
      return 0;
  return n == 6;
}

int
main(int argc, char *argv[])
{
  if (argc != 4 || argv[1][0] < '1' || argv[1][0] > '9' || argv[1][1] != 0
      || strlen(argv[2]) > 100 || !valid_nonce(argv[3])) {
    fprintf(2, "usage: proof <lab> <username> <code>\n");
    fprintf(2, "Copy the exact command from the Moodle question.\n");
    exit(1);
  }
  int lab = argv[1][0] - '0';
  const char *user = argv[2], *nonce = argv[3];

  uint64 ts = time();
  if (ts < 1700000000) {
    fprintf(2, "proof: the clock is not working (time() returned %lu)\n", ts);
    exit(1);
  }

  uint64 e = 0;
  if (evidence(lab, nonce, &e) < 0) {
    fprintf(2, "proof: no token. Fix the problem above, then run proof again.\n");
    exit(1);
  }

  // base = "L<lab>|<user>|<nonce>|<time>|<fingerprint>"
  uint fp = MACHINE_FP;
  char base[180], buf[220], body[80], *p;
  p = putstr(base, "L");
  p = putdec(p, lab);
  p = putstr(p, "|");
  p = putstr(p, user);
  p = putstr(p, "|");
  p = putstr(p, nonce);
  p = putstr(p, "|");
  p = putdec(p, ts);
  p = putstr(p, "|");
  p = puthex4(p, fp >> 16);
  puthex4(p, fp & 0xFFFF);

  uint p1 = H(base) & 0xFFFF;
  p = putstr(buf, base);
  p = putstr(p, "|");
  putdec(p, e);
  uint p2 = H(buf) & 0xFFFFFFFF;

  p = putstr(body, "L");
  p = putdec(p, lab);
  p = putstr(p, "-");
  p = putdec(p, ts);
  p = putstr(p, "-");
  p = puthex4(p, fp >> 16);
  p = puthex4(p, fp & 0xFFFF);
  p = putstr(p, "-");
  p = puthex4(p, p1);
  p = putstr(p, "-");
  p = puthex4(p, p2 >> 16);
  p = putstr(p, "-");
  puthex4(p, p2 & 0xFFFF);

  printf("Copy this whole line into the Moodle answer box:\n\n");
  printf("%s-%c\n", body, CHECK_ALPHABET[H(body) % 32]);
  exit(0);
}
