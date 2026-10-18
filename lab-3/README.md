# Lab 3 — System Calls, Traps, and the Timer

*Operating Systems · Aristotle University of Thessaloniki · Fall 2026*

> **Time:** about 1½ hours of work, at any time while the lab is open (two weeks).
> Sections 1–6 take about 15, 20, 10, 15, 25, and 5 minutes.
> **Help:** ask in [**#OS-Labs-2026**](https://auth-os.zulipchat.com/#narrow/channel/646188-OS-Labs-2026)
> on Zulip, not by private message.
>
> **Before you start**, get this lab's code: `git pull --no-edit upstream main`
> in your `os-lab` directory, then commit and push.
>
> **Tip:** this lab reads a lot of kernel code. If you set up VS Code (see
> the appendix of Lab 2), now is the time to use it.

> [!NOTE]
> **Lecture connection.** Lecture 01's **interrupts** and interrupt cycle
> are `usertrap()` and `scause` in Sections 2–4. The **user and kernel
> modes** and the **mode switch** of lectures 02–03 are what `ecall` and the
> return to user mode do. The timer and `yield()` in Section 4 preview
> **preemptive scheduling** (lecture 09): xv6's scheduler takes turns,
> **round robin**, through the process table. The lock in Section 5 previews
> **mutual exclusion** (lecture 05), the subject of Lab 4.

---

## 1. Read before you run

In Lab 1 you downloaded a script, `setup.sh`, and ran it (in Codespaces, the
codespace ran it for you when it was created). It installed software on your
system, with administrator rights. You trusted it because the
course told you to. Every script you run, whether downloaded, copied from a
web page, or written for you by an AI assistant, can do anything you can
do. Before running one, you should be able to read it and tell roughly what
it does.

A **shell script** is a text file of shell commands, run one after the
other, plus a few constructs for decisions and repetition. Everything you
typed in Lab 1 works in a script.

### 1.1 Five pieces of shell, tried by hand

Try each line in your terminal and compare with the output:

```bash
echo $HOME
x=5
echo "x is $x"
echo "today is $(date +%A)"
```

```text
/home/maria
x is 5
today is Monday
```

- `HOME` is a **variable**; `$HOME` is its value. `x=5` creates one: no
  spaces around `=`.
- `$(...)` runs a command and puts its output in place: **command
  substitution**.

Every command ends with an **exit status**, a number the shell keeps in
`$?`: 0 means success, anything else means failure.

```bash
true;  echo $?
false; echo $?
ls nothere 2>/dev/null || echo "ls failed"
```

```text
0
1
ls failed
```

- `2>/dev/null` sends the error messages (output stream 2) to
  `/dev/null`, a file that discards whatever is written to it.
- `A || B` runs B only if A **failed**. (`A && B` runs B only if A
  succeeded.)

### 1.2 Reading `setup.sh`

Open the script that set up your workbench, with line numbers:

```bash
less -N ~/os-lab/env/setup.sh     # Codespaces: less -N /workspaces/os-lab/env/setup.sh
```

Find each excerpt below with `/` (for example `/set -u` and Enter), read
the lines around it, and answer the question in your head before reading
the explanation.

**`#!/usr/bin/env bash`** (line 1). *What program runs this file?*
The first line, the **shebang**, names the interpreter: `bash`. Lines
starting with `#` are comments.

**`set -u`**. *What does this protect against?*
It makes the script stop with an error if it uses a variable that was
never set, instead of silently using an empty value. Imagine
`rm -rf "$DIR/"` with a misspelled `DIR`.

**`die() { red "ERROR: $*"; ...; exit 1; }`**. *What is this?*
A **function**: a named group of commands. `die` prints an error and ends
the script with exit status 1, failure. `$*` is all the arguments it was
given.

**`[ "$(id -u)" -ne 0 ] || die "Do not run this script with sudo ..."`**.
*When does the script stop here?*
`id -u` prints your user number (try it: usually 1000; for the
administrator, **root**, it is 0). `[ A -ne 0 ]` tests "A is not equal to
0". So: if you are **not** root, carry on; otherwise, `die`. The script
refuses to run as root because it uses `sudo` only where needed. Running
the whole script as root would also create its log file and settings as
root, in the wrong home directory.

**`case "$PWD" in /mnt/*) die ...`**. *Which mistake does this catch?*
`case` compares a value against patterns. `$PWD` is the current directory;
`/mnt/*` matches anything under `/mnt`, the Windows drive in WSL. That is
safety rule 1 from Lab 1, enforced.

**`PKGS=( build-essential ... )`** and **`apt-get install ... "${PKGS[@]}"`**.
*Where is the list of software to install?*
`PKGS` is an **array**, a list of words; `"${PKGS[@]}"` expands to all of
them. To see exactly what the script installs, you only need to read this
list.

**`check "C compiler (gcc)" sh -c "gcc ..."`**. *How does it decide PASS or
FAIL?*
`check` runs the command given after the description and looks at its
exit status: 0 prints PASS, anything else FAIL.

> [!TIP]
> **A checklist for any script before you run it:** What does it install
> or download? Where does it write? Does it use `sudo`, and for what? Does
> it delete anything (`rm`)? Does it download and run *another* script
> (`curl ... | bash`)? If you cannot answer, do not run it yet. Ask.

### 1.3 Your own script

Create `~/greet.sh` with nano:

```bash
#!/usr/bin/env bash
# greet: say hello to someone
echo "Hello, $1! Today is $(date +%A)."
```

`$1` is the first argument given to the script. Make it executable and
run it:

```bash
chmod +x ~/greet.sh
~/greet.sh Maria
```

```text
Hello, Maria! Today is Monday.
```

`chmod +x` sets the e**x**ecutable permission from Lab 1's `ls -l`; the
shebang tells the kernel to run the file with bash. You will write a longer
script, with a loop, in Lab 4.

---

## 2. The system call path

A user program cannot touch the hardware or the kernel's memory; only the
kernel can. When a program needs something only the kernel can do (create
a process, read a file, learn its own pid), it makes a **system call**:
a controlled jump into the kernel, through a single door, and back. In this
section you follow one, `getpid()`, through every step.

```bash
cd ~/os-lab/xv6          # Codespaces: cd /workspaces/os-lab/xv6
make qemu                # build once, then leave xv6 with Ctrl+A, X
```

### 2.1 The user side: `ecall`

`user/user.h` declares `int getpid(void);`, but there is no C code behind
it. The build generates its machine code from `user/usys.pl` into
`user/usys.S`:

```bash
grep -n -A3 "^getpid:" user/usys.S
grep -n "SYS_getpid" kernel/syscall.h
```

```text
89:getpid:
90- li a7, SYS_getpid
91- ecall
92- ret
12:#define SYS_getpid 11
```

Three RISC-V instructions. `li a7, SYS_getpid` puts the system call's
**number**, 11, in register `a7`. **`ecall`** ("environment call") is the
door: the processor switches from **user mode** to **supervisor mode**
(the kernel's mode), and jumps to the address the kernel registered for
traps. When the kernel is done, its answer is in register `a0`, which is
where C expects a function's return value; `ret` returns it to the caller.

### 2.2 The kernel side

```text
 user mode    getpid()  →  li a7, 11 ; ecall
 ─────────────────────────────────┼──────────────────────────────────
 supervisor   uservec       kernel/trampoline.S   save the user's registers
 mode         usertrap()    kernel/trap.c         why are we here? scause 8: a system call
              syscall()     kernel/syscall.c      call syscalls[a7], put the result in a0
              sys_getpid()  kernel/sysproc.c      return myproc()->pid
              prepare_return() kernel/trap.c      get ready to return to user mode
              userret       kernel/trampoline.S   restore the user's registers, sret
 ─────────────────────────────────┼──────────────────────────────────
 user mode    getpid() returns the value in a0
```

Read the two key functions. In `usertrap()` (`grep -n "^usertrap" kernel/trap.c`,
then `nano +LINE kernel/trap.c`):

```c
  if (r_scause() == 8) {
    // system call
    ...
    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;
    ...
    syscall();
  } else if ((which_dev = devintr()) != 0) {
```

The hardware records **why** it entered the kernel in the register
`scause`: 8 means "`ecall` from user mode". (Section 3 shows another
reason, Section 4 a third.) The kernel saved the user's registers in the
process's **trapframe**; it advances the saved program counter past the
4-byte `ecall`, so the program continues after it, and calls `syscall()`
in `kernel/syscall.c`:

```c
  num = p->trapframe->a7;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
  } else {
    printk("%d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
```

`syscalls` is a table of functions, indexed by number: entry 11 is
`sys_getpid`. The result goes into the saved `a0`, which becomes the
program's `a0` when it returns to user mode.

### 2.3 Watching every system call

Make the kernel announce each system call. In `kernel/syscall.c`, add one
line right after `num = p->trapframe->a7;`:

```c
  printk("%d %s: syscall %d\n", p->pid, p->name, num);
```

(`printk` is the kernel's own `printf`.) Run `make qemu`. The boot
messages are now full of lines like `1 init: syscall 16`, mixed with the
text being printed. At the prompt, type `echo hi`:

```text
$ 2 sh: syscall 5
echo hi
2 sh: syscall 5
...                          (one read per character you typed)
2 sh: syscall 1
2 sh: syscall 3
3 sh: syscall 7
3 echo: syscall 16
hi3 echo: syscall 16

3 echo: syscall 2
2 sh: syscall 16
$
```

Look up the numbers in `kernel/syscall.h` (5 `read`, 1 `fork`, 3 `wait`,
7 `exec`, 16 `write`, 2 `exit`) and the story is Lab 2's: the shell
(pid 2) reads your command one character at a time, `fork`s, and `wait`s;
the child (pid 3) `exec`s and becomes `echo` (its name changes in the
trace); `echo` writes `hi` and a newline, and `exit`s; the shell wakes up
and writes the next prompt. Also notice that `init: starting sh` during
boot took one `write` **per character**: xv6's `printf` is simple, not
fast.

Remove your line when you are done:

```bash
git restore kernel/syscall.c
```

### 2.4 Down to the hardware: `time()`

Since Lab 1, `proof` has called `time()`, a system call the course added to
xv6. Follow it yourself, the same way: find its stub in `user/usys.S`, its
number in `kernel/syscall.h`, its entry in the `syscalls` table, and
`sys_time` in `kernel/sysproc.c`:

```c
uint64
sys_time(void)
{
  // Reading TIME_LOW latches TIME_HIGH, so read low first.
  uint64 lo = *(volatile uint32 *)(RTC0 + 0x00);
  uint64 hi = *(volatile uint32 *)(RTC0 + 0x04);
  return ((hi << 32) | lo) / 1000000000ULL;
}
```

`RTC0` (in `kernel/memlayout.h`) is the address of the emulated computer's
**real-time clock**: reading that memory address reads the clock chip's
registers, which count nanoseconds since 1970. Only the kernel has this
address mapped (see `kernel/vm.c`); a user program cannot read the clock
directly. It has to ask, through `ecall`.

> [!NOTE]
> **Optional: the same with a debugger.** gdb can stop the kernel at any
> line and show its variables. Once, allow gdb to read xv6's settings
> file: `echo "set auto-load safe-path /" >> ~/.gdbinit`. Then use **two
> terminals** (Lima: a second Mac terminal with `limactl shell os`; WSL: a
> second Ubuntu window; ChromeOS: a second Terminal tab; Codespaces: the **+**
> in the terminal panel), both
> in the `xv6` directory:
>
> 1. First terminal: `make qemu-gdb`. xv6 waits for the debugger.
> 2. Second terminal: `gdb-multiarch kernel/kernel`, then:
>
> ```text
> (gdb) break kernel/syscall.c:145
> (gdb) continue
> Thread 1 hit Breakpoint 1, syscall () at kernel/syscall.c:145
> (gdb) print num
> $1 = 15
> (gdb) print p->name
> $2 = "init", ...
> (gdb) backtrace
> ```
>
> The very first system call is `init` opening the console (15, `open`).
> `continue` runs to the next one; `quit` leaves gdb. If line 145 is not
> the `if (num > 0 ...` line in your copy, break at that line's number.

---

## 3. Privilege: the kernel protects itself

`ecall` is the only door into the kernel. What happens if a program tries
to get in some other way? The program `user/peek.c` tries two things only
the kernel may do: read the kernel's memory, and execute an instruction
reserved for supervisor mode. Read it (`less user/peek.c`), add
`$U/_peek\` to `UPROGS` in the Makefile as in Lab 2, rebuild with `make qemu`,
and run it:

```text
$ peek
peek: reading the kernel's memory at 0x80000000...
usertrap(): unexpected scause 0xd pid=3
            sepc=0x34 stval=0x80000000
$ peek csr
peek: reading register sstatus...
usertrap(): unexpected scause 0x2 pid=4
            sepc=0x54 stval=0x100025f3
$ echo still alive
still alive
```

Neither attempt got anywhere. In both cases the **processor itself**
refused, and entered the kernel through `usertrap()` as if the program had
made a system call, but with a different reason in `scause`:

| `scause` | Meaning | `stval` holds |
|---|---|---|
| `8` | `ecall` from user mode: a system call (Section 2) | |
| `0xd` (13) | **load page fault**: reading an address the program may not read | the address |
| `0xf` (15) | store page fault: writing such an address | the address |
| `0x2` | **illegal instruction**: not allowed in user mode | the instruction |

For a page fault, `usertrap()` first checks whether the address belongs to
the program's memory but has not been given a page yet (that branch is left
out of the excerpt in Section 2; you will meet it in Lab 5). `0x80000000` is
far outside `peek`'s memory, and an illegal instruction is never legitimate,
so `usertrap()` prints the `unexpected scause` message you just read (find it
in `kernel/trap.c`) and kills the process. The kernel, the shell, and every other process carry on.
`sepc` is the address of the instruction that failed, in the program's own
code.

How does the processor know? It always knows its current **mode**, and an
instruction like `csrr sstatus` is simply not permitted in user mode. And
it checks every memory access against the running process's **page
table**, which describes the addresses that process may use. The kernel's
memory is not in a user process's page table at all. You will look inside
page tables in Lab 5.

> [!NOTE]
> Linux does the same. A Linux program that reads memory it does not own
> is killed with the message `Segmentation fault`, which you may already
> have seen from your own C programs.

---

## 4. The timer takes over

A program enters the kernel when it makes a system call, or when it does
something forbidden. But what about a program that does neither, one that
just computes, forever? `user/spin.c` is such a program:

```c
  volatile uint64 n = 0;
  for (;;)
    n++;
```

Once a processor runs it, nothing in the program will ever give the
processor back. Yet you will see that xv6 keeps going.

### 4.1 Interrupts

Each processor has a **timer**. The kernel sets it to go off about every
tenth of a second (`w_stimecmp(r_time() + 1000000)` in `kernel/trap.c`);
when it does, the processor stops whatever it is running, **in user mode or
not**, and enters the kernel, just as for a system call. That is an
**interrupt**: caused by hardware outside the program, not by the program.
Devices interrupt too: every key you press arrives as an interrupt from the
keyboard's serial port.

`usertrap()` asks `devintr()` which device interrupted; 2 means the timer:

```c
  // give up the CPU if this is a timer interrupt.
  if (which_dev == 2)
    yield();
```

`yield()` marks the current process as ready to run (`RUNNABLE`) and
switches to the **scheduler**, a loop in `kernel/proc.c` that walks through
the process table and picks the next `RUNNABLE` process. So every tenth of
a second, each processor can move on to another process: the processor is
**shared in time slices**, and this is why it is called **preemptive**
scheduling: the kernel takes the processor away, whether the program
cooperates or not.

### 4.2 Watching it

Add `$U/_spin\` to `UPROGS`, run `make qemu`, and start four spinning
processes in the background, more than xv6's three processors:

```text
$ spin &
$ spin &
$ spin &
$ spin &
$ echo still responsive
still responsive
```

The shell still answers. Press **Ctrl + P** a few times, a second apart:

```text
1 sleep  init                1 sleep  init
2 sleep  sh                  2 sleep  sh
4 run    spin                4 run    spin
6 runble spin                6 run    spin
8 run    spin                8 runble spin
10 run   spin                10 run   spin
```

Three spinners are running (`run`), one per processor, and one is waiting
for its turn (`runble`), a different one each time: the timer keeps
rotating them. The shell sleeps until you type; then the keyboard interrupt
wakes it, and the next time slice is its.

Stop the spinners with `kill` and their pids, for example `kill 4`, or just
leave xv6 with **Ctrl + A**, **X**.

### 4.3 Without the timer

Now take preemption away. In `usertrap()` in `kernel/trap.c`, turn **both**
lines into comments:

```c
  // give up the CPU if this is a timer interrupt.
  // if (which_dev == 2)
  //   yield();
```

(Comment out both: with only `yield();` commented, the `if` would apply to
whatever statement comes next.) Run `make qemu` and start **three**
spinners, one per processor. Then try a command:

```text
$ spin &
$ spin &
$ spin &
echo still responsive
```

Nothing happens: the shell is frozen. Press **Ctrl + P**, which the kernel
handles inside the keyboard interrupt:

```text
1 runble init
2 runble sh
7 zombie sh
4 run    spin
6 run    spin
8 run    spin
```

The shell is `runble`: it was waiting for pid 7, the helper that started the
last `spin &`, and pid 7's exit woke it (what you typed just waits in the
keyboard buffer). But no processor will ever be free to run it, because
the spinners never give theirs back, and nothing takes it from them
anymore. `init` is stuck the same way. Meanwhile pid 7 stays a
**zombie**: a process that has exited, but whose parent (the shell) has
not yet called `wait` for it, and cannot run to do so. Leave xv6 with
**Ctrl + A**, **X**, and
undo your change:

```bash
git restore kernel/trap.c
```

Without preemption, one buggy program, or a few, would freeze the whole
computer. Early personal operating systems really worked this way, and
did.

---

## 5. Your own system call

Time to add a system call to xv6: **`getprocs()`**, which returns the number
of processes that exist. The course has already added the user side, the
same way `time()` was added: the number `SYS_getprocs` (24) in
`kernel/syscall.h`, the stub in `user/usys.pl`, and the declaration
`int getprocs(void);` in `user/user.h`. It has also started the kernel
function that will do the counting, `nproc()`, at the end of
`kernel/proc.c`. You will complete the kernel side in three steps.

**Testing.** `proof` doubles as your test program: run it with any name and
code, for example `proof 3 test 123456`. It tells you what is still wrong.
(Only the command from Moodle gives a token you can submit.) Right now:

```text
$ proof 3 test 123456
3 proof: unknown sys call 24
proof: getprocs() returned -1: the kernel does not know this system call yet (check syscall.c)
```

The first line comes from the kernel, from the `else` branch of
`syscall()` you read in Section 2.

### 5.1 The system call function

At the end of `kernel/sysproc.c`, next to `sys_getpid()` and `sys_time()`,
add the function that the system call table will point to. It only asks
`nproc()` for the answer:

```c
// Lab 3: return the number of processes.
uint64
sys_getprocs(void)
{
  return nproc();
}
```

### 5.2 The table entry

In `kernel/syscall.c`, the kernel only knows the functions listed in the
`syscalls` table. Find the lines for `sys_time` and add the same for
`sys_getprocs`, in two places:

```c
extern uint64 sys_time(void);
extern uint64 sys_getprocs(void);     // add this line
```

```c
  [SYS_time]    = sys_time,
  [SYS_getprocs] = sys_getprocs,      // add this line
```

The first line tells the compiler that `sys_getprocs` exists in another
file. The second puts it in the table at position `SYS_getprocs`, 24.
`make qemu` and test:

```text
$ proof 3 test 123456
proof: getprocs() says 0 processes exist, but at least 3 do: init, sh, and proof
```

Progress: the call reaches the kernel and returns. But `nproc()` still
returns 0.

### 5.3 Counting processes

At the end of `kernel/proc.c`, complete `nproc()`. Every process is an
entry in the array `proc`, which has `NPROC` (64) entries; unused entries
have the state `UNUSED`. `procdump()`, just above `nproc()`, walks the same
array to print the Ctrl + P list. Use it as your model.

> [!TIP]
> **C you need here.**
>
> - `struct proc` (in `kernel/proc.h`) groups everything the kernel knows
>   about one process: `state`, `pid`, `name`, its memory, its trapframe…
> - `struct proc *p;` declares a **pointer** to such a struct, and
>   `p->state` is the `state` field of the struct `p` points to.
> - The loop `for (p = proc; p < &proc[NPROC]; p++)` makes `p` point to
>   each entry of the array in turn, from the first to the last.
> - The states are named constants: `UNUSED`, `USED`, `SLEEPING`,
>   `RUNNABLE`, `RUNNING`, `ZOMBIE`. Compare with `!=` or `==`.
>
> **Why the lock?** xv6 runs on three processors at once. While your loop
> looks at an entry, another processor may be changing that same process's
> state. `acquire(&p->lock)` waits until no one else holds that entry's
> lock and takes it; `release(&p->lock)` lets others in. Every piece of
> kernel code that reads or changes `p->state` does the same. (`procdump()`
> does not, because it is only a debugging aid.) Locks are the subject of
> Lab 4.

When it works, `proof` shows the count before, during, and after it creates
some extra processes:

```text
$ proof 3 test 123456
getprocs(): 3 before, 7 with 4 more children, 3 after
```

Save your work:

```bash
git add kernel/proc.c kernel/syscall.c kernel/sysproc.c Makefile
git commit -m "Add getprocs system call"
git push
```

---

## 6. Prove that you did it

Run the command from the Moodle question, as in the previous labs. `proof`
counts processes with **your** `getprocs()` while it creates and removes a
number of children that depends on your attempt's code, and prints a token
only if every count is right:

| `proof` says | Check |
|---|---|
| `unknown sys call 24` / `returned -1` | the table entry in `syscall.c` (5.2), and that you rebuilt |
| `undefined reference to 'sys_getprocs'` (when building) | `sys_getprocs()` in `sysproc.c` (5.1) |
| `says 0 processes exist` | the loop in `nproc()` (5.3) |
| `should have grown by …` | `nproc()` counts every state except `UNUSED` |
| `should be back to where it started` | the same: count all states except `UNUSED` |
| xv6 freezes, or panics in `acquire` | every `acquire(&p->lock)` needs its `release(&p->lock)`, also before any `return` inside the loop |

If xv6 freezes, leave it with **Ctrl + A**, **X**, fix the code, and
`make qemu` again. When everything is right, `proof` prints:

```text
$ proof 3 maria 4f1c92
getprocs(): 3 before, 7 with 4 more children, 3 after
Copy this whole line into the Moodle answer box:

L3-1792365694-09E36ECE-E2AE-A568-3734-F
```

Other programs running in the background (your spinners from Section 4, for
example) do not matter: `proof` only looks at the differences.

### 6.1 Checkpoint

- Which instruction takes a user program into the kernel, and in which
  register does the kernel find the system call number?
- What does `scause` hold, and which values did you see in this lab?
- Why did the shell freeze when the timer no longer called `yield()`?
- Which files did you change to add `getprocs()`, and what does each
  change do?
- Why must `nproc()` hold `p->lock` while it looks at `p->state`?
