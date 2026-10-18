// fork.c: one process becomes two.
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    printf("parent: my pid is %d\n", getpid());

    pid_t pid = fork();          // from here on, two processes run this code
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {              // fork returned 0: we are the child
        printf("child:  my pid is %d, my parent is %d\n", getpid(), getppid());
        return 0;
    }

    // fork returned the child's pid: we are the parent
    printf("parent: I created child %d\n", pid);
    wait(NULL);                  // wait until the child has finished
    printf("parent: my child has finished\n");
    return 0;
}
