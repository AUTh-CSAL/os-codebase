# Operating Systems: lab code

Lab material for course 049, Operating Systems, Department of Electrical
and Computer Engineering, Aristotle University of Thessaloniki.

The labs use [xv6](https://github.com/mit-pdos/xv6-riscv), a small teaching
operating system from MIT, running on an emulated RISC-V computer.

## Labs

| Lab | Topic |
|-----|-------|
| [Lab 1](lab-1/README.md) | Your Linux workbench: terminal, shell, editor, compiler, first boot of xv6 |

**Start with [Lab 1](lab-1/README.md).** It explains how to get a Linux
terminal on Windows, macOS, or a Chromebook, or in your browser with GitHub
Codespaces.

## What is here

| Directory        | Contents                                                     |
|------------------|--------------------------------------------------------------|
| `lab-1/`, …      | The lab instructions                                         |
| `xv6/`           | xv6, with a few additions for this course (see below)        |
| `env/`           | `setup.sh` installs the tools; `lima-os.yaml` is the macOS virtual machine |
| `.devcontainer/` | Configuration for GitHub Codespaces                          |

`xv6/` is [mit-pdos/xv6-riscv](https://github.com/mit-pdos/xv6-riscv) at
commit `f780d78`, plus a `time()` system call, a console fix for pasted
text, and the `proof` program used by the lab quizzes. Run `git log -- xv6`
to see each change separately.

## Help

Ask on the course Zulip. xv6 is © MIT, under the license in
[`xv6/LICENSE`](xv6/LICENSE).
