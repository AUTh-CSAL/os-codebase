#!/usr/bin/env bash
# Install the tools for the Operating Systems labs.
#
#   curl -fsSLO https://raw.githubusercontent.com/AUTh-CSAL/os-codebase/main/env/setup.sh
#   bash setup.sh
#
# Works on Ubuntu 24.04 (WSL, Lima, Codespaces) and Debian 12/13 (ChromeOS).
# Safe to run more than once. A copy of everything it prints is saved in
# ~/os-setup.log: attach that file when asking for help on Zulip.

set -u
LOG="$HOME/os-setup.log"
exec > >(tee "$LOG") 2>&1

red()   { printf '\033[1;31m%s\033[0m\n' "$*"; }
green() { printf '\033[1;32m%s\033[0m\n' "$*"; }
step()  { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
die()   { red "ERROR: $*"; red "Ask on Zulip and attach the file $LOG"; exit 1; }

# ---------------------------------------------------------------- sanity checks
step "Checking your system"

[ "$(id -u)" -ne 0 ] || die "Do not run this script with sudo or as root. Run: bash setup.sh"
command -v apt-get >/dev/null || die "This is not an Ubuntu/Debian system (no apt-get)."

. /etc/os-release
echo "System:    $PRETTY_NAME ($(uname -m))"
case "$ID" in
  ubuntu|debian) ;;
  *) red "Warning: untested system '$ID'; continuing anyway." ;;
esac

if grep -qi microsoft /proc/version 2>/dev/null; then
  if [ -z "${WSL_INTEROP:-}" ] && ! grep -qi wsl2 /proc/version; then
    die "You are on WSL 1. In PowerShell run: wsl --set-version Ubuntu-24.04 2"
  fi
  case "$PWD" in
    /mnt/*) die "You are inside the Windows drive ($PWD). Run 'cd ~' first, then try again." ;;
  esac
  echo "Platform:  WSL 2"
elif [ -n "${CODESPACES:-}" ]; then
  echo "Platform:  GitHub Codespaces"
elif [ -d /mnt/chromeos ]; then
  echo "Platform:  ChromeOS"
else
  echo "Platform:  Linux"
fi

# Installing needs administrator rights; ask for the password once, up front.
echo
echo "The next step installs software and needs your Linux password"
echo "(nothing appears on screen while you type it; that is normal)."
sudo -v || die "Could not get administrator rights (wrong password?)."

# ------------------------------------------------- man pages on minimal images
# Minimal Ubuntu images (Lima, containers) strip all manual pages. Bring them
# back, but keep the other space-saving exclusions (documentation, translations).
EXCL=/etc/dpkg/dpkg.cfg.d/excludes
RESTORE_MAN=no
if [ -f "$EXCL" ] && grep -q '^path-exclude=/usr/share/man/' "$EXCL"; then
  step "Re-enabling manual pages (this system was minimized)"
  sudo sed -i 's|^path-exclude=/usr/share/man/|#&|' "$EXCL"
  RESTORE_MAN=yes
fi
if dpkg-divert --list /usr/bin/man 2>/dev/null | grep -q man.REAL; then
  sudo rm -f /usr/bin/man
  sudo dpkg-divert --quiet --local --rename --remove /usr/bin/man
  RESTORE_MAN=yes
fi

# ------------------------------------------------------------------- packages
step "Updating the package list"
export DEBIAN_FRONTEND=noninteractive
sudo -E apt-get update -q || die "apt-get update failed. Check your internet connection."

has_pkg() { apt-cache policy "$1" 2>/dev/null | grep -q 'Candidate: [0-9]'; }

# Newer Debian splits the RISC-V emulator into its own package.
QEMU=qemu-system-misc
has_pkg qemu-system-riscv && QEMU=qemu-system-riscv

PKGS=(
  build-essential gdb-multiarch          # gcc, make, debugger
  gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu "$QEMU"   # xv6 toolchain
  git gh nano less curl ca-certificates file  # everyday tools
  man-db manpages manpages-dev           # manual pages, incl. C library
  strace python3 bc                      # later labs; bc: xv6 Makefile
)

step "Installing course tools (a few hundred MB; may take several minutes)"
sudo -E apt-get install -y -q --no-install-recommends "${PKGS[@]}" \
  || die "Package installation failed."

# Also when an earlier run lifted the exclusion but the pages are still
# missing, so that running the script again always repairs them.
if [ "$RESTORE_MAN" = yes ] || ! man -w 2 fork >/dev/null 2>&1; then
  step "Restoring manual pages of basic commands"
  # Packages that were already installed when man pages were still excluded
  # (container images ship some of these) have lost their pages: reinstall.
  sudo -E apt-get install -y -q --reinstall \
    coreutils findutils grep sed tar gzip diffutils less procps bash util-linux \
    man-db manpages manpages-dev git make \
    >/dev/null || die "Could not reinstall basic packages."
  sudo mandb -q
fi

# ------------------------------------------------------------------ self-test
step "Checking that everything works"
FAIL=0
check() {  # check "description" command...
  if "${@:2}" >/dev/null 2>&1; then green "  PASS  $1"; else red "  FAIL  $1"; FAIL=1; fi
}

T=$(mktemp -d)
printf '#include <stdio.h>\nint main(void){puts("ok");return 0;}\n' > "$T/hello.c"
printf 'int f(int x){return x+1;}\n' > "$T/rv.c"

check "C compiler (gcc)"            sh -c "gcc -o '$T/hello' '$T/hello.c' && '$T/hello'"
check "make"                        make --version
check "RISC-V cross-compiler"       riscv64-linux-gnu-gcc -c -march=rv64gc -mabi=lp64d -ffreestanding -o "$T/rv.o" "$T/rv.c"
check "RISC-V emulator (QEMU)"      qemu-system-riscv64 --version
check "debugger (gdb-multiarch)"    gdb-multiarch --version
check "git"                         git --version
check "GitHub CLI (gh)"             gh --version
check "manual pages (man ls)"       sh -c "man -w ls"
check "C library manual (man 2 fork)" sh -c "man -w 2 fork"
rm -rf "$T"

QV=$(qemu-system-riscv64 --version | sed -n 's/.*version \([0-9]*\.[0-9]*\).*/\1/p')
echo
echo "gcc $(gcc -dumpversion), riscv64 gcc $(riscv64-linux-gnu-gcc -dumpversion), qemu $QV"

echo
if [ "$FAIL" -eq 0 ]; then
  green "ALL CHECKS PASSED: your workbench is ready."
else
  die "Some checks failed (see FAIL lines above)."
fi
