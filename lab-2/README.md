# Lab 2 — Processes

*Operating Systems · Aristotle University of Thessaloniki · Fall 2026*

> **Time:** about 1½ hours of work, at any time while the lab is open (two weeks).
> Sections 1–4 take about 25, 20, 30, and 5 minutes.
> **Help:** ask in [**#OS-Labs-2026**](https://auth-os.zulipchat.com/#narrow/channel/646188-OS-Labs-2026)
> on Zulip, not by private message.

> [!NOTE]
> **Lecture connection (lecture 03, Processes).** `struct proc` in
> `kernel/proc.h` is the **process control block** of lecture 03: one per
> process, with its state, pid, memory, and saved registers. Its `state` is
> one of `UNUSED`, `USED`, `SLEEPING`, `RUNNABLE`, `RUNNING`, `ZOMBIE`, which
> Ctrl + P shows as `sleep`, `runble`, `run`, `zombie`: lecture 03's
> **Blocked**, **Ready**, **Running**, and **Exit** states. `fork`, `exec`,
> `exit`, and `wait` are **process creation and termination**, the way Unix
> does them.

---

## 1. Your own repository

From this lab on, you change xv6 and write programs of your own. **Git**
keeps a history of your project: every time you **commit**, it records a
snapshot of all your files, with a message saying what changed. **GitHub**
keeps a copy of that history online. Together they give you:

- a **backup**: if your laptop dies, your work is on GitHub;
- **sync**: the same work on your laptop and in Codespaces;
- a **history**: when something breaks, you can see what you changed, and
  go back;
- **course updates**: new code for each lab reaches you through Git.

You will work in a **private** copy of the course repository. Only you (and
anyone you explicitly invite) can see it.

```text
 AUTh-CSAL/os-codebase           your Linux              you/os-lab
 (course repository)   ──pull──▶  ~/os-lab  ◀──push/pull──▶ (your private copy)
      "upstream"                                            "origin"
```

### 1.1 Make your private copy

1. Sign in at <https://github.com> and open <https://github.com/new/import>.
2. **Your source repository details → URL:**
   `https://github.com/AUTh-CSAL/os-codebase.git`
3. **Repository name:** `os-lab`.
4. Select **Private**. This matters: in a public repository, anyone could
   copy your solutions.
5. Click **Begin import**. After a minute or two you get a page with the
   address `https://github.com/YOUR-USERNAME/os-lab`, marked **Private**.

> [!NOTE]
> GitHub's import page sometimes shows an **error page** (500), or its
> button stays greyed out at *Preparing import…*. The import usually
> finishes anyway. Wait two minutes, then open your repository list
> (your picture, top right → **Your repositories**). If `os-lab` is there
> and shows the `xv6`, `env`, `lab-1`, and `lab-2` folders, it worked. If it
> is missing or empty, delete it (**Settings** → bottom of the page →
> **Delete this repository**) and import again. If that fails too, ask on
> Zulip.

### 1.2 Get your copy onto your Linux

Your Lab 1 copy of the course code has nothing in it you need, and two xv6
directories are easy to confuse. Replace it with your own.

**Paths A, B, C (WSL, Lima, ChromeOS).** First, let your terminal act on
your behalf on GitHub. In your Linux terminal:

```bash
gh auth login -h github.com -p https -w
```

It prints a one-time code such as `ABCD-1234` and asks you to press Enter to
open your browser. If no browser opens (in Lima or ChromeOS it will not),
open <https://github.com/login/device> yourself. Enter the code
and click **Authorize**. If the terminal asks *Authenticate Git with your
GitHub credentials?*, answer **Y**. Check that it worked:

```bash
gh auth status
```

Then remove the Lab 1 copy and get yours. Read the `rm` line twice before
pressing Enter:

```bash
cd ~
rm -rf ~/os-codebase
gh repo clone os-lab
cd os-lab
```

**Path D (Codespaces).** A codespace can only save to the repository it was
created from, so create a new one from **your** copy:

1. On `https://github.com/YOUR-USERNAME/os-lab`, click **Code** →
   **Codespaces** → **Create codespace on main**. Setup runs again
   (about 5 minutes); meanwhile, continue reading.
2. Delete your Lab 1 codespace at <https://github.com/codespaces>
   (**⋯** → **Delete**): you no longer need it, and it uses your storage.
3. In the new codespace's terminal you are already in your copy,
   `/workspaces/os-lab`. You do not need `gh auth login`.

### 1.3 Tell Git who you are

Every commit records its author. Use the name and e-mail of your GitHub
account, so that GitHub links your commits to you:

```bash
git config --global user.name "Maria Papadopoulou"
git config --global user.email "maria@ece.auth.gr"
git config --global pull.rebase false
git config --global core.editor nano
```

The last two lines choose the simplest way to combine course updates with
your own changes, and make Git use nano when it needs you to write a
message. You do this once per computer.

### 1.4 Connect to the course repository

Your copy knows where it came from: `origin`, your repository on GitHub.
Add the course repository too, under the name `upstream`:

```bash
git remote add upstream https://github.com/AUTh-CSAL/os-codebase.git
git remote -v
```

```text
origin    https://github.com/maria/os-lab.git (fetch)
origin    https://github.com/maria/os-lab.git (push)
upstream  https://github.com/AUTh-CSAL/os-codebase.git (fetch)
upstream  https://github.com/AUTh-CSAL/os-codebase.git (push)
```

Whenever the course announces an update, bring it in with:

```bash
git pull --no-edit upstream main
```

Right now there is nothing new, so Git reports `Already up to date.`

### 1.5 The everyday cycle

Create a file for your notes, and ask Git what it sees:

```bash
echo "# My OS lab notes" > NOTES.md
git status
```

```text
On branch main
Your branch is up to date with 'origin/main'.

Untracked files:
  (use "git add <file>..." to include in what will be committed)
	NOTES.md
```

Git sees a new file that it does not track yet. Saving a change in Git takes
three steps:

| Command | What it does |
|---|---|
| `git add FILE` | choose which changes go into the next snapshot (**staging**) |
| `git commit -m "MESSAGE"` | record the snapshot, with a message saying what changed |
| `git push` | upload your new commits to GitHub |

```bash
git add NOTES.md
git commit -m "Add my notes file"
git push
```

```text
[main 59a7e40] Add my notes file
 1 file changed, 1 insertion(+)
 create mode 100644 NOTES.md
```

Reload your repository's page on GitHub: `NOTES.md` is there. Now change
the file, and look at the change before committing it:

```bash
echo "- Lab 2: started" >> NOTES.md
git diff
```

```text
--- a/NOTES.md
+++ b/NOTES.md
@@ -1 +1,2 @@
 # My OS lab notes
+- Lab 2: started
```

Lines starting with `+` were added, lines starting with `-` removed. For
files Git already tracks, `commit -a` stages all their changes for you:

```bash
git commit -am "Update notes"
git push
git log --oneline -3
```

```text
fe0f8c4 Update notes
59a7e40 Add my notes file
...                       (the course's latest commit)
```

`git log` lists the history, newest first: your two commits, on top of the
course's.

> [!TIP]
> **`git status` is always safe** and tells you where you are and what to
> do next. Run it whenever you are unsure. And push at the end of every
> session: work that exists only on one computer is not backed up.
>
> **Working on more than one computer?** Start every session with `git pull`,
> which brings in what you pushed from the other one. If `git push` is ever
> *rejected*, it means GitHub has commits you do not have yet: run `git pull`,
> then push again.

> [!NOTE]
> **Learn more Git.** This section covers the few commands you need.
> To go further: the lecture on version control from MIT's
> [*Missing Semester*](https://missing.csail.mit.edu/2020/version-control/),
> the interactive [*Learn Git Branching*](https://learngitbranching.js.org/),
> [GitHub Skills](https://skills.github.com/), and the free book
> [*Pro Git*](https://git-scm.com/book).

### 1.6 Checkpoint

- What is the difference between `origin` and `upstream`?
- What do `git add`, `git commit`, and `git push` each do?
- Which command shows what you changed but have not committed yet?
- How do you get course updates into your copy?

---

## 2. Processes on Linux

A **process** is a program while it runs: its machine code, its memory, the
files it has open, and the processor state the kernel saves and restores
when it switches between processes. The kernel gives every process a
number, its **process ID** (**pid**).

### 2.1 The processes you already have

Your shell is a process too. `$$` is its pid, and `ps` lists the
processes running in your terminal:

```bash
echo $$
ps
```

```text
10438
    PID TTY          TIME CMD
  10438 pts/1    00:00:00 bash
  10439 pts/1    00:00:00 ps
```

`ps` sees itself: to run `ps`, the shell created a new process. Every
process is created by another one, its **parent**, so all processes form a
tree. See the whole tree (`-e` every process, `-f` full details,
`--forest` drawn as a tree):

```bash
ps -ef --forest | less
```

In `less`, type `/ps -ef` and Enter to find your own command, then look
up the tree: `ps` was started by your `bash`, which was started by the
program that opened your terminal, and so on up to pid 1, the first
process the kernel starts. The `PPID` column is the **parent's** pid.
Quit with `q`.

**Background jobs.** A command followed by `&` runs **in the background**:
the shell starts it and gives you the prompt back immediately.

```bash
sleep 100 &
jobs
kill %1
```

```text
[1] 10440
[1]+  Running                 sleep 100 &
```

`[1]` is the job number and `10440` the pid. `kill %1` sends job 1 a
**signal** asking it to terminate; press Enter once more, and the shell
reports `[1]+  Terminated  sleep 100`.

### 2.2 `fork`: one process becomes two

Programs create processes with the **system call** `fork`. Get the Lab 2
programs and compile the first one:

```bash
cd ~/os-lab/lab-2        # Codespaces: cd /workspaces/os-lab/lab-2
less fork.c
gcc -Wall -o fork fork.c
./fork
```

```text
parent: my pid is 10391
parent: I created child 10392
child:  my pid is 10392, my parent is 10391
parent: my child has finished
```

`fork` creates a copy of the calling process: same program, same memory
contents, same open files, at the same point in the code. **It returns
twice**, once in each process:

- in the **child**, `fork` returns **0**;
- in the **parent**, it returns the **child's pid**.

That return value is the only way the two copies can tell who is who, which
is why every `fork` is followed by an `if`. Run `./fork` a few times: the
order of the middle two lines can change, because after `fork` the kernel
decides which process runs first.

`wait(NULL)` makes the parent sleep until its child finishes. Find out what
happens without it: delete the `wait(NULL);` line, recompile, run.

```bash
nano fork.c
gcc -Wall -o fork fork.c
./fork
```

```text
parent: my pid is 10444
parent: I created child 10445
parent: my child has finished
child:  my pid is 10445, my parent is 10444
```

The parent now claims its child has finished before the child has even
printed. The shell's prompt may appear before the child's line, and the
child may report a different parent (often 1, or a `systemd` process): when a parent exits
first, the kernel gives its orphans to another process. Programs that
create processes must `wait` for them.

Undo your change: `git restore` puts back the last committed version of a
file.

```bash
git restore fork.c
gcc -Wall -o fork fork.c
```

> [!NOTE]
> **A copy means a copy.** Try `./fork | cat`. The first line now appears
> **twice**. When output goes to a pipe instead of the terminal, `printf`
> keeps text in a memory buffer and writes it later. The buffer still held
> the first line when `fork` copied the process, so both copies wrote it
> out.

### 2.3 `exec`: becoming another program

`fork` gives you a copy of the same program. To run a *different* program,
a process calls `exec`, which **replaces** its program with another one: it
keeps the pid, but its code and memory are thrown away and loaded fresh
from the new program's file. If it succeeds, `exec` never returns.

```bash
less exec.c
gcc -Wall -o exec exec.c
./exec
```

```text
total 40
-rwxrwxr-x 1 maria maria 70584 Oct 19 00:20 exec
-rw-r--r-- 1 maria maria   703 Oct 19 00:20 exec.c
...
parent: child 10398 exited with status 0
```

The child became `ls -l`; the parent waited, and `wait` gave it the
child's **exit status**: 0 means success. Change `"ls", "ls"` to
`"lss", "lss"`, recompile, and run: now `exec` fails, the line after it
runs, and the child exits with status 127. Then `git restore exec.c`.

**This is how your shell runs every command:** it `fork`s, the child
`exec`s the program you typed, and the shell `wait`s for it before showing
the next prompt. With `&`, it skips the `wait`.

### 2.4 Watching the system calls

`strace` shows the system calls a program makes: every request it sends
to the kernel. Watch the process-related ones (`-f` follows children too):

```bash
strace -f -e trace=process ./exec
```

```text
execve("./exec", ["./exec"], ...) = 0
clone(child_stack=NULL, flags=...|SIGCHLD, ...) = 10421
[pid 10420] wait4(-1,  <unfinished ...>
[pid 10421] execve("/usr/local/sbin/ls", ["ls", "-l"], ...) = -1 ENOENT (No such file or directory)
[pid 10421] execve("/usr/local/bin/ls", ["ls", "-l"], ...) = -1 ENOENT (No such file or directory)
[pid 10421] execve("/usr/sbin/ls", ["ls", "-l"], ...) = -1 ENOENT (No such file or directory)
[pid 10421] execve("/usr/bin/ls", ["ls", "-l"], ...) = 0
...
[pid 10421] exit_group(0)               = ?
<... wait4 resumed>[{WIFEXITED(s) && WEXITSTATUS(s) == 0}], 0, NULL) = 10421
parent: child 10421 exited with status 0
```

Read it top to bottom:

- `execve("./exec")`: the process `strace` started became your program.
- `clone(...) = 10421`: your `fork`. On Linux, the C library implements
  `fork` with a more general system call, `clone`.
- `wait4`: the parent waits.
- `execve(... "ls" ...)`: the child tries the directories where commands
  live, one by one, until it finds `/usr/bin/ls`. (`execlp` does this
  search; the `p` stands for *path*.)
- `exit_group(0)`: the child exits with status 0, and `wait4` returns its
  pid.

`man 2 fork`, `man 2 execve`, `man 2 wait` describe each of these calls.

### 2.5 Save your work

```bash
git status
```

Git shows nothing to commit: you restored both files, and the compiled
programs are listed in `lab-2/.gitignore`, so Git ignores them. Nothing to
push yet; that comes with your xv6 program in Section 3.

### 2.6 Checkpoint

- What does `fork` return in the parent, and in the child?
- Why did the parent's message lie when you removed `wait`?
- After a successful `exec`, what happens to the code that follows it?
- How does the shell run a command you type? What changes with `&`?
- Which system call does Linux use to implement `fork`?

---

## 3. Processes in xv6

Now the same ideas inside xv6, where you can read every line of the code
involved.

```bash
cd ~/os-lab/xv6          # Codespaces: cd /workspaces/os-lab/xv6
```

### 3.1 How xv6's shell runs a command

Open xv6's shell, `user/sh.c`, at its main loop:

```bash
nano +160 user/sh.c
```

```c
  while (getcmd(buf, sizeof(buf)) >= 0) {
    ...
    if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == ' ') {
      // Chdir must be called by the parent, not the child.
      ...
    } else {
      if (fork1() == 0)
        runcmd(parsecmd(cmd));
      wait(0);
    }
  }
```

It is the pattern from Section 2: read a line, `fork` (here `fork1`, which
is `fork` plus an error check), let the child run the command, `wait`.
Now look at what the child does for a simple command (exit nano, then
`nano +75 user/sh.c`):

```c
  case EXEC:
    ...
    exec(ecmd->argv[0], ecmd->argv);
    fprintf(2, "exec %s failed\n", ecmd->argv[0]);
```

The child `exec`s the program; if `exec` returns, it failed. This is where
the message `exec proof failed` came from in Lab 1, when a program was
missing.

> [!NOTE]
> **Why is `cd` different?** Every process has its own current directory.
> If a child process ran `cd`, it would change *its own* directory and then
> exit: the shell would stay where it was. So the shell runs `cd` itself.

### 3.2 Your first xv6 program

Create `user/hello.c` with nano:

```c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("hello from xv6, my pid is %d\n", getpid());
  exit(0);
}
```

It looks like the Linux C you know, with two differences. xv6 has no
standard C library: `user/user.h` declares the system calls and the few
library functions xv6 provides (`printf`, `strlen`, `atoi`, …). And by
convention an xv6 program ends with `exit(0)`, the system call that ends the
process. (This xv6 also calls `exit` for you if `main` returns, but older
versions crash.)

The Makefile lists the programs to build and put on xv6's disk, in a
variable called `UPROGS`. Open the Makefile, search for it with
**Ctrl + W** `UPROGS` Enter (in Codespaces in a browser: **F6**, since
Ctrl + W closes the tab), and add two lines at the end of the list, one
for your program and one for the next one:

```make
	$U/_proof\
	$U/_hello\
	$U/_pingpong\
```

Each line ends with `\`, which continues the list on the next line. Save,
then build, boot, and run your program:

```bash
make qemu
```

```text
$ hello
hello from xv6, my pid is 3
```

Its pid is 3: process 1 is `init`, 2 is the shell, and the shell `fork`ed
3 to run your command. Leave xv6 (**Ctrl + A**, **X**) and save your work:

```bash
git add user/hello.c Makefile
git commit -m "Add hello program"
git push
```

### 3.3 pingpong: talking through pipes

A **pipe** is a one-way channel the kernel provides between processes:
what one process **writes** into one end, another **reads** from the other
end. `pipe(p)` creates one and returns its two ends in `p[0]` (read) and
`p[1]` (write). The ends are **file descriptors**, the same kind of number
a process uses for an open file, so they work with `read` and `write`.
Because a child inherits its parent's file descriptors, a pipe created
before `fork` connects the two.

`pingpong` uses two pipes, one for each direction:

```text
                 p2c: parent → child
   parent  ──────────────────────────▶  child
  (pid 3)  ◀──────────────────────────  (pid 4)
                 c2p: child → parent
```

Read the program, which is already in your repository:

```bash
less user/pingpong.c
```

It is complete except for four lines, marked `TODO`. Run it as it is
(`make qemu`, then `pingpong 41`):

```text
$ pingpong 41
4: received ping 0
3: received pong 0
```

The child prints 0, because nobody sent it anything yet. Your job is to
fill in the four TODOs so that the parent's number travels to the child,
and the child's answer back (your pids may differ: each command you run
takes new ones):

```text
$ pingpong 41
4: received ping 41
3: received pong 42
```

> [!TIP]
> **C you need here.** Each TODO is a single call:
>
> ```c
> read(FD, &VARIABLE, sizeof(VARIABLE));
> write(FD, &VARIABLE, sizeof(VARIABLE));
> ```
>
> - `FD` is the pipe end: `p2c[0]`, `p2c[1]`, `c2p[0]`, or `c2p[1]`. To
>   send from parent to child, the parent **writes** into `p2c[1]` and the
>   child **reads** from `p2c[0]`.
> - `&x` is the **address** of the variable `x`: `read` needs to know
>   *where* to put the bytes it receives, and `write` where to take them
>   from.
> - `sizeof(x)` is the size of `x` in bytes: 4 for an `int`.
>
> `read` waits until there is something to read. That is how the two
> processes take turns.

Edit, `make qemu`, test, repeat. If `pingpong` hangs, a process is waiting
to `read` something that nobody writes: most often the child waits for the
number because the parent never wrote it (TODO 3). Leave xv6 with
**Ctrl + A**, **X**, and check which TODOs read from and write to which pipe.
(`proof` runs your `pingpong`, so it hangs the same way.)

When it works, save your work:

```bash
git add user/pingpong.c
git commit -m "Complete pingpong"
git push
```

---

## 4. Prove that you did it

The Moodle quiz for this lab has a proof of work question, as in Lab 1.
This time `proof` runs **your** `pingpong` with a number made from the
attempt's code, and checks what it prints:

```text
$ proof 2 maria 4f1c92
pingpong 184659 printed:
5: received ping 184659
4: received pong 184660

Copy this whole line into the Moodle answer box:

L2-1792358737-09E36ECE-511E-3527-A2E1-F
```

If something is wrong, `proof` says what instead of printing a token:

| `proof` says | Check |
|---|---|
| `cannot run pingpong` | `$U/_pingpong\` is in `UPROGS`, and you ran `make qemu` after adding it |
| `expected exactly two lines` | `pingpong` prints only the two lines, in the format shown |
| `the pong line must be printed by the parent` | the parent prints `pong`, the child `ping` |
| `the child did not receive …` | TODO 1 and TODO 3 |
| `the parent did not receive …` | TODO 2 and TODO 4 |

As in Lab 1, tokens are unique to you, your quiz attempt, and your
installation. Finally, make sure everything is pushed:

```bash
git status
git log --oneline -3
```

`git status` should say your branch is up to date with `origin/main`, and
your commits should be on GitHub.

### 4.1 Checkpoint

- Why does the shell run `cd` itself instead of in a child process?
- What are `p[0]` and `p[1]` after `pipe(p)`?
- Why must the pipes be created **before** `fork`?
- In `read(fd, &x, sizeof(x))`, why `&x` and not `x`?
- Why could `pingpong` hang, and how do you get out of xv6 when it does?

---

## Appendix: a programming editor

nano is enough to change a few lines. To *read* a code base like xv6, you
want a programming editor: search across all files, jump from a function
call to its definition, see several files side by side. Which one you use
is up to you; what matters is that you learn one well. Plenty of good
tutorials exist, so here are only the starting points.

- **Visual Studio Code** is the most common choice, and the one Codespaces
  runs in your browser (nothing to install there). Otherwise, install it
  on your **laptop** from <https://code.visualstudio.com>, not inside
  Linux: the editor window runs on your laptop, while the files, the
  compiler, and the terminal stay in Linux, where they work. (Your laptop
  cannot open your Linux files directly: with Lima they live on the
  virtual machine's own disk.) An extension connects the two:
  - **WSL:** install the *WSL* extension, then in your Ubuntu terminal run
    `code ~/os-lab`.
  - **Lima:** install the *Remote - SSH* extension, add the line
    `Include ~/.lima/os/ssh.config` to the file `~/.ssh/config` on your
    Mac, then connect to the host `lima-os`.
  - **ChromeOS:** the exception. ChromeOS cannot run VS Code itself, so
    download the `.deb` package for Linux and install it in the Linux
    terminal with `sudo apt install ./code_*.deb`.
- **VSCodium** (<https://vscodium.com>) is the same editor without
  Microsoft's telemetry and branding. Microsoft's remote extensions do not
  work in it; use *Open Remote - SSH* instead.
- **Emacs**, **Vim / Neovim**, or **mg** run inside the terminal, on every
  path, with nothing else to set up. They take longer to learn and repay
  it for a lifetime.

Whatever you choose, learn three things first: how to search across all
files, how to jump to a definition, and how to open a file by name.
