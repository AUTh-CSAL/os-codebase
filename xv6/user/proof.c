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

// Lab-specific evidence: something only a working xv6 for that lab produces.
static int
evidence(int lab, const char *nonce, uint64 *e)
{
  struct stat st;

  switch (lab) {
  case 1:
    // Lab 1: you built and booted xv6, and it can read its file system.
    if (stat("README", &st) < 0)
      return -1;
    *e = st.size;
    return 0;
  default:
    return -1;
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

  uint64 e;
  if (evidence(lab, nonce, &e) < 0) {
    fprintf(2, "proof: lab %d is not complete yet in this xv6\n", lab);
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
