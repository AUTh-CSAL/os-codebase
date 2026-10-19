# Lab 1 — Your Linux Workbench

*Operating Systems · Aristotle University of Thessaloniki · Fall 2026*

> **Time:** about 1½ hours of work, at any time while the lab is open (two weeks).
> Section 1 (≈30 min) is mostly waiting for downloads: start it early.
> Sections 2–5 take about 30, 10, 10, and 10 minutes.
> **Help:** ask on the class Zulip (see [Getting help](#getting-help)), not by private message.

> [!NOTE]
> **Lecture connection (lectures 01–02).** The three layers in Section 1 are
> lecture 02's picture of an operating system: a layer between the hardware
> and your programs, which here you can run inside another one. QEMU, which
> imitates a RISC-V processor, carries out lecture 01's **instruction cycle**
> (fetch an instruction, execute it, check for interrupts) in software. And
> `file` in Section 4 shows that a program is machine code for one particular
> kind of processor.

---

## 1. Get a Linux terminal

Every lab in this course runs on **Linux**. You will not replace or modify
the operating system on your laptop. Instead you will get a small, separate
Linux system that lives *next to* it, and that you can delete with a single
command when the course is over.

You will use that Linux system through a **terminal**: a window where you
type commands instead of clicking. If you have never used one, that is
normal. Section 2 starts from zero.

### The big picture: three computers in one

By the end of this lab you will have three systems running inside each
other, each with its own command prompt:

```text
┌───────────────────────────────────────────────────────────────┐
│ 1. Your laptop: Windows, macOS, or ChromeOS                   │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │ 2. Linux (Ubuntu): a virtual machine on your laptop     │  │
│  │    (in Codespaces: on a GitHub server instead)          │  │
│  │  ┌───────────────────────────────────────────────────┐  │  │
│  │  │ 3. xv6: a small teaching OS, running on a RISC-V  │  │  │
│  │  │    computer that the program QEMU imitates        │  │  │
│  │  └───────────────────────────────────────────────────┘  │  │
│  └─────────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────────┘
```

Commands only work in the layer they belong to, so **always check the prompt
before typing**:

| Layer | The prompt looks like | How to get in | How to get out |
|---|---|---|---|
| 1. Laptop | `PS C:\Users\maria>` (Windows)<br>`maria@MacBook ~ %` (Mac) | — | — |
| 2. Linux | `maria@laptop:~$`<br>(the part before `:` varies: `lima-os`, `penguin`, …)<br>Codespaces: `@maria ➜ /workspaces/os-codebase (main) $` | Section 1 shows how, for your path | `exit` |
| 3. xv6 | just `$`, nothing before it | `make qemu` (Section 5) | **Ctrl + A**, then **X** |

Almost all your work happens in layer 2. Layer 3 is the operating system you
will study and modify; its shell is deliberately tiny, so Tab, ↑, and the
other conveniences of Section 2 do not work there.

> [!NOTE]
> **Two different tricks are at work here.** Layer 2 is **virtualized**: Linux
> runs directly on your laptop's processor, with hardware support that keeps
> it separate from your own system, so it is almost as fast as the real
> thing. Layer 3 is **emulated**: xv6 expects a RISC-V processor, which your
> laptop does not have, so QEMU imitates one in software, instruction by
> instruction. That is slower, but it works on any laptop. You will meet both
> ideas again in the lectures.

### 1.1 Pick your path

There are four equally good paths. They all end in the same place: a
terminal running Linux, with the same tools, behaving the same way.

| You have…                              | Use path                                  |
|----------------------------------------|-------------------------------------------|
| A Windows 10/11 laptop                 | [A: WSL](#path-a--windows-wsl)            |
| A Mac (Intel or Apple Silicon)         | [B: Lima](#path-b--macos-lima)            |
| A Chromebook                           | [C: ChromeOS Linux](#path-c--chromebook-linux-development-environment) |
| Any computer with a web browser — including the **Vergina** lab PCs, a borrowed computer, or a laptop where path A/B/C failed | [D: Codespaces](#path-d--github-codespaces-browser) |

You may use more than one path: for example, WSL at home and Codespaces in
Vergina. In Lab 2 you will learn Git, which keeps your work in sync between
them.

> [!NOTE]
> **Why does it not matter what processor my laptop has?**
> Later in this lab you will run *xv6*, a small teaching operating system,
> inside an **emulator** called QEMU. The emulator imitates a complete
> RISC-V computer in software, so xv6 behaves identically on an Intel
> laptop, an Apple M-series Mac, or a cloud server.

### 1.2 Everyone: create a GitHub account

All four paths need a GitHub account (Codespaces runs on GitHub, and from
Lab 2 on you will store your work there).

1. Go to <https://github.com/signup> and create an account. Use your
   **university e-mail** (`…@ece.auth.gr` or `…@auth.gr`).
   Pick a username you would not mind a future employer seeing.
2. Apply for the free **GitHub Student Developer Pack** at
   <https://education.github.com/pack>. It raises your free Codespaces
   allowance. Approval can take a few days; you can continue meanwhile.

### 1.3 Safety rules — read before installing anything

These rules ensure nothing you do in this course can harm your laptop or
your personal files.

1. **Work only inside your Linux home directory.** In Section 2 you will
   learn that it is called `~`. On Windows, the Linux side can also see your
   Windows drive under `/mnt/c`. **Never work there**: a mistaken delete
   command in `/mnt/c` deletes real Windows files. (It is also about ten
   times slower.)
2. **`sudo` means "as administrator".** You will need it only during
   installation, exactly where the instructions say so. If a later
   instruction, website, or chatbot tells you to use `sudo` and this lab did
   not, stop and ask on Zulip.
3. **Never change BIOS/UEFI settings, partition disks, or set up
   dual-boot** for this course. If an installation path asks for any of
   this, abandon it and use path D instead.
4. **Everything is removable.** Each path has a single uninstall command,
   listed in [1.6](#16-how-to-remove-everything-later).

---

### Path A — Windows (WSL)

WSL ("Windows Subsystem for Linux") is a Microsoft component that runs a
real Linux system inside Windows.

**Requirements:** Windows 11, or Windows 10 version 22H2. About 5 GB of free
disk space.

**A1. Open an administrator terminal.**
Right-click the **Start** button and choose **Terminal (Admin)**
(on Windows 10: **Windows PowerShell (Admin)**). Answer **Yes** when Windows
asks whether to allow changes.

**A2. Install WSL with Ubuntu.** Type exactly this and press Enter:

```powershell
wsl --install -d Ubuntu-24.04
```

Wait for the download to finish. If you are asked to restart Windows, do so.

**A3. Create your Linux user.** After the installation (or after the
restart) a window titled *Ubuntu* opens and asks for:

- a **username**: lowercase letters only, no spaces (e.g. `maria`);
- a **password**, typed twice.

> [!IMPORTANT]
> **When you type the password, nothing appears on screen — no dots, no
> stars.** This is normal for Linux terminals. Type it and press Enter.
> Choose a password you will remember: you need it every time you use
> `sudo`. It does not have to match your Windows password.

If the *Ubuntu* window did not open by itself, open the **Start** menu, type
`Ubuntu`, and open **Ubuntu 24.04 LTS**.

**A4. Check that you are on WSL 2.** In the administrator terminal from A1
(not in Ubuntu), type:

```powershell
wsl -l -v
```

The line for `Ubuntu-24.04` should show `2` in the `VERSION` column. If it
shows `1`, run `wsl --set-version Ubuntu-24.04 2`.

**From now on**, to open your Linux terminal: **Start → Ubuntu 24.04 LTS**
(or open *Terminal* and choose *Ubuntu-24.04* from the ⌄ menu next to the
tab). You do not need an administrator terminal again.

<details>
<summary><b>If it went wrong</b></summary>

- **Error `0x80370102` or a message about "virtualization" / "Virtual
  Machine Platform".** Your laptop has hardware virtualization switched off
  in its firmware. Do **not** change firmware settings (safety rule 3). Use
  [path D](#path-d--github-codespaces-browser).
- **`wsl --install` only prints a help text.** WSL is already present. Run
  `wsl --install -d Ubuntu-24.04` again; if Ubuntu is also already
  installed, just open it from the Start menu.
- **"The requested operation requires elevation".** You skipped step A1:
  the terminal must be opened as administrator.
- **Anything else:** copy the full error text and post it on Zulip
  (see [Getting help](#getting-help)).

</details>

---

### Path B — macOS (Lima)

Lima is a free, open-source tool that runs a small Linux virtual machine on
a Mac with a single command.

**Requirements:** macOS 13 (Ventura) or newer. About 5 GB of free disk space.

**B1. Open the Mac terminal.** Press **⌘ Space**, type `Terminal`, press
Enter.

**B2. Install Homebrew** (a package manager for macOS), unless you already
have it. Typing `brew --version` tells you: if it prints a version number,
skip to B3. Otherwise paste this line and press Enter:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

It asks for your **Mac password** (nothing appears while you type; that is
normal) and may install Apple's *Command Line Tools*, which takes several
minutes. When it finishes, it prints **"Next steps"** with two or three
commands to run: **run them**, then close and reopen Terminal.

**B3. Install Lima and create the Linux machine:**

```bash
brew install lima
limactl start --name=os --tty=false https://raw.githubusercontent.com/AUTh-CSAL/os-codebase/main/env/lima-os.yaml
```

The first command may end with a note about `lima-additional-guestagents`;
ignore it, you do not need them.
The second command downloads a minimal Ubuntu 24.04 (about 250 MB) and
starts it. It ends with a line containing `READY`.

**B4. Enter Linux:**

```bash
limactl shell os
cd ~
```

The prompt changes. You are now inside Ubuntu. The `cd ~` moves you to your
Linux home directory: always do this after entering.

> [!NOTE]
> Lima lets Linux *read* your Mac home folder but not change it. That is
> intentional: your Mac files are safe from anything you do inside Linux.

**From now on**, to open your Linux terminal: open Terminal, then run
`limactl shell os` followed by `cd ~`. After restarting your Mac, run
`limactl start os` first.

<details>
<summary><b>If it went wrong</b></summary>

- **`brew: command not found` after installing Homebrew.** You skipped the
  "Next steps" commands at the end of B2. Scroll up, run them, reopen
  Terminal.
- **`limactl start` fails or hangs for more than 15 minutes.** Run
  `limactl delete -f os` and repeat B3 once. If it fails again, post the
  output on Zulip and use path D meanwhile.

</details>

---

### Path C — Chromebook (Linux development environment)

ChromeOS has a built-in Linux system (Debian) that you only need to switch
on.

**C1.** Open **Settings → About ChromeOS → Developers → Linux development
environment → Turn on.** (On some versions: **Settings → Advanced →
Developers**.)

**C2.** Choose a username (lowercase, no spaces) and a disk size of **at
least 10 GB**. Click **Install** and wait.

**C3.** A terminal window opens when it finishes. Later, open it from the
launcher: **Terminal → penguin**.

> [!NOTE]
> Path C gives you **Debian 12**, a close relative of the Ubuntu used on the
> other paths. Every command in this course works the same on both.

<details>
<summary><b>If it went wrong</b></summary>

- **There is no "Developers" or "Linux development environment" entry.**
  The Chromebook is managed by a school or company that has disabled it, or
  it is too old. Use [path D](#path-d--github-codespaces-browser).

</details>

---

### Path D — GitHub Codespaces (browser)

A *codespace* is a Linux computer in GitHub's cloud that you use through
your web browser. Nothing is installed on your machine, so this path works
anywhere, including the **Vergina** lab PCs. The course codespace comes with
every tool already installed.

**D1.** Sign in at <https://github.com>. Any up-to-date browser works; if
the page misbehaves (for example, it goes blank when you paste), reopen
your codespace in Chrome, Edge, or Firefox.

**D2.** Open the course repository:
<https://github.com/AUTh-CSAL/os-codebase>

**D3.** Click the green **Code** button → **Codespaces** tab → **Create
codespace on main**. The first start takes 2–5 minutes; later starts take
seconds.

**D4.** An editor (Visual Studio Code) opens in your browser. The
**terminal** is the panel at the bottom. If you do not see it, press
**Ctrl + `** (the key above Tab), or use the ☰ menu → **Terminal → New
Terminal**.

**From now on**, to return: go to <https://github.com/codespaces> and click
your codespace. **Do not** create a new one each time: your files live in
the one you created.

> [!WARNING]
> **Codespaces has a free monthly allowance** (about 60 hours per month on
> the default 2-core machine, more with the Student Pack). Your codespace
> stops automatically after 30 minutes without activity, but to save hours,
> stop it yourself when you finish: <https://github.com/codespaces> → **⋯**
> next to your codespace → **Stop codespace**.
>
> **An unused codespace is deleted after 30 days**, with any files in it.
> From Lab 2 on you will save your work to GitHub with `git push`, so
> nothing is lost. Until then, Lab 1 has nothing you cannot recreate.

> [!CAUTION]
> **On shared computers (Vergina, libraries): sign out of GitHub when you
> leave.** Anyone who sits down after you can otherwise use your account.

---

### 1.4 Checkpoint: is your terminal working?

Open your Linux terminal (whichever path you took). You should see a line
ending in `$`. That is the **prompt**: Linux is waiting for a command.
Type each of the following lines and press Enter after each:

```bash
whoami
uname -m
grep PRETTY_NAME /etc/os-release
```

You should see something like:

```text
maria
x86_64
PRETTY_NAME="Ubuntu 24.04.1 LTS"
```

- `whoami` prints your Linux username (in Codespaces it is `vscode`).
- `uname -m` prints your processor type: `x86_64` (Intel/AMD) or `aarch64`
  (ARM: Apple M-series, Snapdragon, many Chromebooks). Either is fine; see
  the note in 1.1.
- The last line names your Linux system: Ubuntu 24.04, or Debian 12 on a
  Chromebook.

**Write down your `uname -m` output. You will need it for the quiz.**

If all three commands printed something similar, your terminal works.

### 1.5 Install the course tools

One script installs everything the labs need: the C compiler, Git, a
text editor, the manual pages, and the RISC-V compiler and emulator for xv6.
Copy these two lines into your Linux terminal and press Enter:

```bash
cd ~ && curl -fsSLO https://raw.githubusercontent.com/AUTh-CSAL/os-codebase/main/env/setup.sh
bash setup.sh
```

> [!TIP]
> **Copy and paste in the terminal:** In WSL and ChromeOS, paste with
> **right-click** or **Ctrl + Shift + V**. On a Mac, use **⌘ V**. In
> Codespaces, use **Ctrl + V** (**⌘ V** on a Mac); if the browser asks for
> permission to paste, allow it. Plain **Ctrl + V** does *not* paste in
> most Linux terminals.

The script asks for your Linux password once (on ChromeOS and Codespaces it
does not ask). It then downloads about 200 MB, which takes 2–10 minutes
depending on your connection. It prints a lot of text; you can ignore it until
the end, which should look like this:

```text
==> Checking that everything works
  PASS  C compiler (gcc)
  PASS  make
  PASS  RISC-V cross-compiler
  PASS  RISC-V emulator (QEMU)
  PASS  debugger (gdb-multiarch)
  PASS  git
  PASS  GitHub CLI (gh)
  PASS  manual pages (man ls)
  PASS  C library manual (man 2 fork)

ALL CHECKS PASSED: your workbench is ready.
```

If you see `FAIL` or `ERROR` lines, run `bash setup.sh` once more: a
dropped connection is the usual cause, and the script is safe to repeat. If
it fails again, post on Zulip and attach the file `os-setup.log` that the
script saved in your home directory.

Section 1 is done. 🎉

### 1.6 How to remove everything later

When the course is over (or if you want to start again from scratch):

| Path         | Remove with                                                                 |
|--------------|-----------------------------------------------------------------------------|
| A: WSL       | In PowerShell: `wsl --unregister Ubuntu-24.04` (deletes the Linux system and all files in it) |
| B: Lima      | In Mac Terminal: `limactl delete -f os` and `limactl prune` (frees the download cache), then optionally `brew uninstall lima` |
| C: ChromeOS  | Settings → Developers → Linux development environment → **Remove**          |
| D: Codespaces| <https://github.com/codespaces> → **⋯** → **Delete**                        |

---

## 2. Talking to the shell

The program that reads what you type in the terminal is the **shell**
(ours is called *bash*). It reads a command, runs it, prints the result,
and waits for the next one. Everything in this course, from compiling code
to booting xv6, starts with typing a command into the shell.

Do this section **at the keyboard**: type every command yourself, and
compare what you see with the expected output. Your username and some
details (dates, sizes) will differ; that is fine.

### 2.1 The prompt and your first commands

Open your Linux terminal and go to your home directory:

```bash
cd ~
```

The line where you type is the **prompt**. It usually shows your username,
the computer's name, and where you are, and ends in `$`:

```text
maria@laptop:~$
```

In this lab, commands are shown **without** the prompt: type only the part
after `$`. Try these, pressing Enter after each:

```bash
date
echo Hello, Linux
whoami
```

```text
Sun Oct 18 18:32:51 EEST 2026
Hello, Linux
maria
```

Most commands have the same shape: a **command name**, then **options**
that change how it behaves (they start with `-`), then **arguments**: the
things it should work on.

```text
ls   -l   /etc
│    │    └── argument: which directory to list
│    └─────── option: long format
└──────────── command name
```

Linux is **case-sensitive**: `ls` works, `LS` and `Ls` do not. The same
holds for file names: `Notes.txt` and `notes.txt` are two different files.

> [!TIP]
> **Four keys that save you hours:**
> - **Tab** completes command and file names. Type `ec` and press Tab: the
>   shell completes `echo`. If nothing happens, press Tab twice to see all
>   possibilities.
> - **↑ / ↓** scroll through the commands you typed before. Press ↑, edit,
>   press Enter to run it again.
> - **Ctrl + C** stops the running command and gives you the prompt back.
>   Use it whenever something seems stuck.
> - **Ctrl + L** (or the command `clear`) clears the screen.

### 2.2 Where am I? Files and directories

Linux keeps files in **directories** (what Windows and macOS call
*folders*), organized as a tree that starts at the **root directory**,
written `/`. Every user has a **home directory** for their own files. Yours
is `/home/` followed by your username (in Codespaces: `/home/vscode`).

```text
/                     the root of the whole tree
├── bin/              programs, such as ls and date
├── etc/              system settings
├── home/
│   └── maria/        your home directory, also written ~
│       └── ...
├── tmp/              temporary files
└── usr/              more programs, libraries, manual pages
```

At every moment, the shell is "standing" in one directory: the **current
working directory**. These commands tell you where you are and what is
there:

| Command   | Meaning                                    |
|-----------|--------------------------------------------|
| `pwd`     | **p**rint **w**orking **d**irectory        |
| `ls`      | **l**i**s**t the contents of a directory   |
| `ls -l`   | long list: permissions, owner, size, date  |
| `ls -a`   | **a**ll: also show hidden files            |

```bash
pwd
ls
ls -a
ls -l /etc/passwd
```

```text
/home/maria
os-setup.log  setup.sh
.  ..  .bash_logout  .bashrc  .profile  os-setup.log  setup.sh
-rw-r--r-- 1 root root 1330 Oct 18 16:57 /etc/passwd
```

Two things to notice in `ls -a`:

- Names that start with a dot, such as `.bashrc`, are **hidden files**.
  They usually hold settings. Plain `ls` does not show them.
- Every directory contains two special entries: `.` means **this
  directory**, and `..` means **the directory above** (the parent).

### 2.3 Moving around: `cd` and paths

`cd` (**c**hange **d**irectory) moves you to another directory. Where to
go is given as a **path**, written in one of two ways:

- An **absolute path** starts with `/` and is read from the root:
  `/home/maria/playground`. It means the same thing wherever you are.
- A **relative path** does not start with `/` and is read from where you
  are now: `playground/notes`, `..`, `../code`.

Three shortcuts:

| Path | Means                               |
|------|-------------------------------------|
| `~`  | your home directory                 |
| `.`  | the current directory               |
| `..` | the parent directory (one level up) |

Practice them. Run `pwd` after each `cd` to see where you ended up:

```bash
cd /etc
pwd
cd ..
pwd
cd ~
pwd
```

```text
/etc
/
/home/maria
```

`cd` with no argument also takes you home. When you are lost, `cd ~` and
`pwd` bring you back to known ground.

### 2.4 Creating directories and files

Now build a small directory tree to practice on. `mkdir` (**m**a**k**e
**dir**ectory) creates directories; `-p` creates the missing parents too,
and does not complain if they already exist.

```bash
cd ~
mkdir -p playground/notes playground/code
cd playground
ls
```

```text
code  notes
```

The simplest way to put text into a file is `echo` with a **redirection**:
`>` sends a command's output into a file instead of the screen. It creates
the file, or **replaces** its contents if it already exists. `>>` adds to
the end of the file instead. `cat` prints a file.

```bash
echo "Operating Systems lab notes" > notes/todo.txt
echo "- install Linux: done" >> notes/todo.txt
cat notes/todo.txt
```

```text
Operating Systems lab notes
- install Linux: done
```

> [!WARNING]
> `>` overwrites without asking. `echo hi > notes/todo.txt` would replace
> both lines with a single `hi`. Use `>>` when you mean "add".

### 2.5 Copying, moving, and deleting

| Command             | Meaning                                              |
|---------------------|------------------------------------------------------|
| `cp SOURCE DEST`    | **c**o**p**y a file                                  |
| `cp -r SOURCE DEST` | copy a directory and everything in it (**r**ecursive)|
| `mv SOURCE DEST`    | **m**o**v**e a file or directory; also how you **rename** |
| `rm FILE`           | **r**e**m**ove (delete) a file                       |
| `rm -r DIR`         | remove a directory and everything in it              |
| `rmdir DIR`         | remove a directory, only if it is empty              |

```bash
cp notes/todo.txt notes/todo-backup.txt
ls notes
mv notes/todo-backup.txt code/
ls -R
```

`ls -R` lists **R**ecursively: the directory and everything below it.

```text
.:
code  notes

./code:
todo-backup.txt

./notes:
todo.txt
```

Now rename the backup, then delete it and the empty directory:

```bash
mv code/todo-backup.txt code/old.txt
ls code
rm code/old.txt
rmdir code
ls
```

```text
old.txt
notes
```

> [!CAUTION]
> **There is no Recycle Bin.** `rm` deletes immediately and permanently,
> without asking, and `rm -r` does the same to a whole directory tree.
> Before pressing Enter on an `rm` command, read it once more. Never run
> `rm` with `sudo`, and never on paths you do not recognize.

### 2.6 Reading files: `cat`, `less`, `head`, `tail`

`cat` is fine for short files. For long ones, use **`less`**, which shows
one screen at a time:

```bash
less /etc/services
```

Inside `less`:

| Key              | Action                           |
|------------------|----------------------------------|
| Space / b        | next / previous page             |
| ↓ / ↑            | one line down / up               |
| `/` word Enter   | search for *word*; `n` for next  |
| `q`              | **quit** back to the shell       |

If you only need the beginning or the end of a file, `head` and `tail` show
its first or last 10 lines; `-n 3` shows 3 instead:

```bash
head -n 3 /etc/os-release
```

```text
PRETTY_NAME="Ubuntu 24.04.5 LTS"
NAME="Ubuntu"
VERSION_ID="24.04"
```

### 2.7 Getting help: `man`

Every command has a **manual page**. `man` opens it in `less`, so the same
keys work (`q` to quit):

```bash
man ls
```

A manual page starts with a **SYNOPSIS** showing how to call the command,
followed by a **DESCRIPTION** of every option. Look up what `ls -t` and
`ls -h` do, and try them.

Manual pages are grouped into **sections**. Section 1 holds commands,
section 2 holds **system calls**: the functions through which a program asks
the operating system kernel for a service. You will read many of these in
this course. For example, `man 2 fork` explains how a program creates a new
process. When a name exists in several sections, give the section number:

```bash
man 2 write
```

Many commands also print a short summary with `--help`, for example
`ls --help`. Searching the web works too, but check that the answer is for
Linux, not for Windows or macOS.

### 2.8 Combining commands: the pipe `|`

The **pipe** `|` sends the output of one command into the input of the
next. Small commands combine into useful ones:

| Command       | Meaning                                        |
|---------------|------------------------------------------------|
| `wc -l`       | **w**ord **c**ount: with `-l`, count lines     |
| `grep WORD`   | keep only the lines that contain *WORD*        |

How many programs are installed in `/usr/bin`, and which ones have `zip` in
their name?

```bash
ls /usr/bin | wc -l
ls /usr/bin | grep zip
```

```text
809
bunzip2
bzip2
bzip2recover
gunzip
gzip
streamzip
zipdetails
```

Your numbers and names may differ. The pipe is not magic: the shell starts
both programs and connects the output of the first to the input of the
second. In Lab 2 you will see exactly how, using the `pipe` system call.

### 2.9 When things go wrong

Error messages are short and usually say exactly what is wrong. Read them:

```bash
ls nothere
cd nothere
```

```text
ls: cannot access 'nothere': No such file or directory
bash: cd: nothere: No such file or directory
```

| You see                         | Usually means                                    |
|---------------------------------|--------------------------------------------------|
| `No such file or directory`     | a typo in the name, or you are in the wrong directory: check with `pwd` and `ls` |
| `command not found`             | a typo in the command name, or the program is not installed |
| `Permission denied`             | you are outside your home directory, or the file is not yours |
| a `>` prompt instead of `$`     | you opened a quote `"` and did not close it: press Ctrl + C |
| nothing happens, no prompt      | the command is still running or waiting for input: press Ctrl + C |

**Spaces separate arguments.** A file named `my notes.txt` looks like two
arguments, `my` and `notes.txt`. Either put quotes around the name
(`cat "my notes.txt"`) or, better, never use spaces in file names: use
`my-notes.txt`.

### 2.10 Clean up and checkpoint

Delete the practice directory. Check the command twice before pressing
Enter:

```bash
cd ~
rm -r playground
ls
```

`playground` is gone. You should now be able to answer, without looking
back:

- What is the difference between `/home/maria/notes` and `notes`?
- Which command shows where you are? Which shows hidden files?
- How do you rename a file? How do you get out of `less`?
- What is the difference between `>` and `>>`?
- Which manual section holds system calls?

**Command summary**

| Command | Purpose | Command | Purpose |
|---|---|---|---|
| `pwd` | where am I | `cat` | print a file |
| `ls -la` | list, with details and hidden files | `less` | read a long file (`q` quits) |
| `cd PATH` | change directory | `head` / `tail` | start / end of a file |
| `mkdir -p` | create directories | `man` | manual page |
| `cp` / `cp -r` | copy | `>` / `>>` | output to file: replace / append |
| `mv` | move or rename | `\|` | pipe output into the next command |
| `rm` / `rm -r` | delete, permanently | `grep` / `wc -l` | filter lines / count lines |

---

## 3. Editing files with nano

To write programs you need a **text editor**. You will use **nano**, an
editor that runs inside the terminal. It works the same on every computer
and every path in this course, and it lists its most important keys at the
bottom of the screen. (In Lab 2 you will also set up Visual Studio Code, for
reading large amounts of code.)

### 3.1 Turn on line numbers

Compilers report errors by line number, so it helps to see them. nano reads
its settings from the hidden file `~/.nanorc`. Add one with the `>>` you
learned in Section 2:

```bash
echo "set linenumbers" >> ~/.nanorc
```

### 3.2 Your first file

Create a directory for this lab, and open a new file in nano:

```bash
mkdir -p ~/lab1
cd ~/lab1
nano hello.c
```

The terminal turns into the editor. Type this small C program (or paste it:
see the tip in 1.5). Use the arrow keys to move around; the mouse does not
move the cursor.

```c
#include <stdio.h>

int main(void)
{
    printf("Hello from %s!\n", "Linux");
    return 0;
}
```

At the bottom of the screen, nano lists its shortcuts. `^` means
**Ctrl**, so `^X` means **Ctrl + X**. On a Mac, too, use the **Ctrl** key,
not ⌘.

| Keys                  | Action                                           |
|-----------------------|--------------------------------------------------|
| **Ctrl + O**, Enter   | save ("write **O**ut"); Enter confirms the name |
| **Ctrl + X**          | exit; if there are unsaved changes, nano asks: **Y** to save, then Enter |
| **Ctrl + K** / **Ctrl + U** | cut the current line / paste it back      |
| **Ctrl + W**          | search ("**w**here is")                          |
| **Ctrl + G**          | help                                             |

Save with **Ctrl + O** and Enter, then exit with **Ctrl + X**. Back at the
prompt, check that the file is there:

```bash
ls -l
cat hello.c
```

### 3.3 Jumping to a line

To open a file with the cursor already at a given line, put `+` and the line
number before the file name. You will use this in Section 4 to go straight
to the line a compiler error points at:

```bash
nano +5 hello.c
```

The cursor is on the `printf` line. Change `Linux` to your own name, save,
and exit.

> [!TIP]
> **nano disappeared and the shell says `Stopped`?** You pressed
> **Ctrl + Z**, which pauses a program instead of quitting it. Type `fg`
> ("foreground") and press Enter to get nano back, then exit properly with
> **Ctrl + X**.

---

## 4. Compiling with gcc and make

Your processor cannot run `hello.c` directly: it is text for humans. A
**compiler** translates it into **machine code** for your processor. On
Linux the standard C compiler is **gcc**.

### 4.1 Compile and run

```bash
cd ~/lab1
gcc -Wall -o hello hello.c
ls -l
```

```text
-rwxrwxr-x 1 maria maria 70320 Oct 18 19:08 hello
-rw-rw-r-- 1 maria maria    94 Oct 18 19:08 hello.c
```

- `gcc hello.c` compiles the file;
- `-o hello` names the **o**utput file `hello` (without it, gcc picks the
  odd name `a.out`);
- `-Wall` turns on **all** the useful **w**arnings. Always use it: warnings
  point at code that compiles but is probably wrong.

The new file `hello` is a program. The `x` in `-rwxrwxr-x` means it is
e**x**ecutable. Run it:

```bash
./hello
```

```text
Hello from Maria!
```

Why `./hello` and not just `hello`? The shell only looks for commands in a
fixed list of system directories, not in the current one. `./hello` gives
the exact path: "the file `hello` in this directory (`.`)". Typing just
`hello` gives `command not found`.

Which processor is this program for? Ask the `file` command:

```bash
file hello
```

```text
hello: ELF 64-bit LSB pie executable, ARM aarch64, version 1 (SYSV), ...
```

You will see `ARM aarch64` or `x86-64`, matching the `uname -m` you wrote
down in Section 1. **ELF** is the format of programs on Linux, and also on
xv6.

### 4.2 Reading a compiler error

Everybody makes typing mistakes; the skill is reading what the compiler
says about them. Make one on purpose: open `hello.c` at line 5 and delete
the `;` at the end of the `printf` line. Save and exit.

```bash
nano +5 hello.c
```

Compile again:

```bash
gcc -Wall -o hello hello.c
```

```text
hello.c: In function ‘main’:
hello.c:5:40: error: expected ‘;’ before ‘return’
    5 |     printf("Hello from %s!\n", "Maria")
      |                                        ^
      |                                        ;
    6 |     return 0;
      |     ~~~~~~
```

Read it from the top:

- `hello.c:5:40` means **file** `hello.c`, **line** 5, **column** 40. Every
  gcc message starts with this location.
- `error:` means compiling failed. (A `warning:` would still produce a
  program, but tells you something looks wrong.)
- Then the explanation, and the line itself with a `^` under the spot. Here
  gcc even suggests the fix: a `;`.

When there are several errors, **fix the first one first**: one mistake
often confuses the compiler into reporting more errors further down.

Now a trap: run the program.

```bash
./hello
```

```text
Hello from Maria!
```

It still runs! Compiling failed, so gcc did not touch the old `hello`, and
you are running the **previous** version. Whenever a program does not
behave as your code says, first ask: did I recompile, and did it succeed?

Fix the error with `nano +5 hello.c`, compile again, and check that gcc
prints nothing. On success, gcc is silent.

### 4.3 Automating builds with make

Remembering to recompile, with the right options, is exactly the kind of
work computers are for. **make** reads a file named `Makefile` that lists,
for each file to build, what it is made from and how:

```make
hello: hello.c
	gcc -Wall -o hello hello.c
```

This **rule** says: the **target** `hello` is built from `hello.c` (its
**dependency**), using the command on the next line (the **recipe**).

> [!IMPORTANT]
> The recipe line must start with a **Tab** character, not spaces. If it
> starts with spaces, make stops with `Makefile:2: *** missing separator.
> Stop.` Copying from a web page usually turns Tabs into spaces, so create
> the Makefile with this command, where `\t` stands for the Tab:
>
> ```bash
> printf 'hello: hello.c\n\tgcc -Wall -o hello hello.c\n' > Makefile
> ```

Create the Makefile with the command in the box, check it with
`cat Makefile`, and run make twice:

```bash
make
make
```

```text
gcc -Wall -o hello hello.c
make: 'hello' is up to date.
```

The first time, make runs the recipe and prints the command. The second
time it does nothing: `hello` is newer than `hello.c`, so nothing changed
since the last build. Edit `hello.c` (change the greeting), and run `make`
again: it notices the change and recompiles. **make rebuilds exactly what
is out of date, and nothing else.**

That is all you need to know about make for now. xv6, which you build in
the next section, has a Makefile with rules for the kernel and every
program, and `make qemu` is one target that builds whatever has changed and
then starts the emulator.

### 4.4 Checkpoint

- What do `-o` and `-Wall` do?
- In `hello.c:12:7: error: ...`, what are 12 and 7?
- Compiling failed, but `./hello` still runs. Why?
- Why must you type `./hello` instead of `hello`?
- What does `make` do when you run it twice without changing anything?

---

## 5. Meet xv6

**xv6** is a small operating system written at MIT for teaching. It is a
modern rewrite of Unix Version 6 from 1975, the ancestor of Linux and
macOS. Its whole kernel is about 6,600 lines of C, small enough to read in
full. (The Linux kernel has over 30 million.) In the coming labs you will
read parts of it and change them.

### 5.1 Get the code

The course code is on GitHub. `git clone` downloads a copy of it; you will
learn Git properly in Lab 2.

**Paths A, B, C (WSL, Lima, ChromeOS):**

```bash
cd ~
git clone https://github.com/AUTh-CSAL/os-codebase.git
cd os-codebase/xv6
```

**Path D (Codespaces):** the code is already there. Just go to it:

```bash
cd /workspaces/os-codebase/xv6
```

Look around:

```bash
ls
ls kernel
ls user
```

`kernel/` holds the operating system itself: `proc.c` manages processes,
`vm.c` virtual memory, `fs.c` the file system. `user/` holds the programs
that run *on* xv6, such as `ls.c` and `sh.c`, its shell.

### 5.2 Boot xv6

```bash
make qemu
```

The first time, make compiles the kernel and all the programs (a few
seconds to a minute), builds a disk image with them, and starts QEMU.
Among the build messages, one says `warning: kernel/kernel has a LOAD
segment with RWX permissions`: it is expected, and you can ignore it. The
last lines are xv6's own boot messages:

```text
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$
```

(A *hart* is a RISC-V processor core; QEMU gives xv6 three of them.)

> [!IMPORTANT]
> **You are now in layer 3** (see [the big picture](#the-big-picture-three-computers-in-one)).
> The prompt is just `$`. This is xv6's own tiny shell: **Tab, ↑, and `man`
> do not work here**, so type commands in full. To leave xv6, press
> **Ctrl + A**, then **X**.

Try a few commands. They look like the Linux ones, but they are xv6's own,
much simpler programs:

```text
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2441
cat            2 3 35448
...
$ echo hello xv6
hello xv6
$ wc README
48 336 2441 README
```

Now press **Ctrl + P**. The kernel prints its list of processes:

```text
1 sleep  init
2 sleep  sh
```

Process 1, `init`, is the first program the kernel starts; it started
process 2, the shell `sh`, which is waiting for you to type. Both are
*sleeping*: waiting for something to happen. Processes are the subject of
Lab 2.

Leave xv6 with **Ctrl + A**, then **X**. You are back in Linux.

### 5.3 One program, two processors

In Linux, ask `file` about xv6's `ls` program, then try to run it:

```bash
file user/_ls
./user/_ls
```

```text
user/_ls: ELF 64-bit LSB executable, UCB RISC-V, RVC, double-float ABI, ...
bash: ./user/_ls: cannot execute binary file: Exec format error
```

`_ls` is machine code for **RISC-V**, built by a special compiler
(`riscv64-linux-gnu-gcc`) that runs on your processor but produces code for
a different one: a **cross-compiler**. Your laptop's processor cannot run
it; only QEMU's imitation RISC-V computer can. Compare with
`file ~/lab1/hello` from Section 4.

### 5.4 Prove that you did it

The Moodle quiz for this lab contains a **proof of work** question. It
shows you a command made for you and for that quiz attempt, such as

```text
proof 1 maria 4f1c92
```

1. Boot xv6 (`make qemu` in the `xv6` directory).
2. **Copy** the command from Moodle and **paste** it at the xv6 `$` prompt.
3. xv6 prints a line such as
   `L1-1792336609-09E36ECE-C75A-8B81-201A-2`. Copy the **whole** line,
   paste it into the Moodle answer box, and press **Check**.

> [!WARNING]
> **Each token is unique to you, to your quiz attempt, and to your
> computer.** It records when and on which installation it was made. A
> token made for someone else is rejected, and tokens from one installation
> submitted by more than one student are reviewed by the instructors. Using
> more than one installation yourself (for example, your laptop and
> Codespaces) is fine.

<details>
<summary><b>If it went wrong</b></summary>

- **`exec proof failed`.** xv6 cannot find the program. Did you run
  `make qemu` in `os-codebase/xv6`? Check with `ls proof` at the xv6
  prompt.
- **`usage: proof <lab> <username> <code>`.** The command is incomplete.
  Copy it again from Moodle, all of it.
- **Moodle says "a typo".** Copy and paste the token; do not retype it.
- **Moodle says "a different user or a different attempt".** You ran an
  older command. Every new quiz attempt has a new code: copy the command
  shown in the attempt you are answering.
- **Moodle says "outside the period this lab is open".** Your computer's
  clock is wrong. Fix the date and time in your laptop's settings, then
  restart xv6.

</details>

### 5.5 Done

Lab 1 is complete. You have:

- a Linux workbench with the course tools;
- the basic commands of the shell, and an editor;
- compiled C with gcc and make;
- booted an operating system that you will take apart in the next labs.

In **Lab 2** you will set up Git and VS Code, save your own copy of xv6 on
GitHub, and see how processes are born, run, and die: first on Linux, then
inside xv6.

---

## Getting help

Join the course Zulip at <https://auth-os.zulipchat.com/join/xttzhr6exabpzkplk7mctezo/>
and ask in the channel
[**#OS-Labs-2026**](https://auth-os.zulipchat.com/#narrow/channel/646188-OS-Labs-2026).

- **Ask in the public stream, not by private message.** Your classmates
  probably have the same problem, and the answer helps everyone.
  **Do not send private messages to the instructors** unless the issue is
  personal (health, special arrangements, grades).
- **Help others.** If you solved a problem someone else is asking about,
  answer them.
- **A good question includes:**
  1. which path you are on (A, B, C, or D) and which lab section;
  2. the exact command you typed;
  3. the **full error message, pasted as text**: not a phone photo of the
     screen. To paste on Zulip, wrap it in triple back-quotes: ```` ``` ````.
