// exec.c: the child replaces itself with another program.
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        // Replace this process's program with "ls -l". If it works,
        // execlp never returns: the code below it is gone.
        execlp("ls", "ls", "-l", NULL);
        perror("execlp");        // reached only if exec failed
        return 127;
    }

    int status;
    wait(&status);               // wait for the child, and get its exit status
    printf("parent: child %d exited with status %d\n", pid, WEXITSTATUS(status));
    return 0;
}
