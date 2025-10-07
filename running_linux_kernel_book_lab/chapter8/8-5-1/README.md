# Process management 

## Chapter 8 Labs 

### fork and clone system call 

(1) Use fork() to create a child process. Both the parent and the child should print a message so we can identify which is which.

(2) Use clone() to create a child. Consider two cases:

      - Process-style (separate address space)

	Flags: SIGCHLD (no CLONE\_VM, no CLONE\_THREAD).

	The parent and child do not share global variables (they each have their own copy via copy-on-write).

	If the parent exits first, the child continues running and is reparented to PID 1 (init/systemd). 

      - Thread-style (shared address space)

	Flags: CLONE\_VM | CLONE\_FS | CLONE\_FILES | CLONE\_SIGHAND | CLONE\_THREAD

	(with CLONE\_THREAD the low 8 bits—the “exit signal”—must be 0, so don’t OR SIGCHLD here).

	The parent and child share the same address space, so they see the same global variables.

	If the creating thread calls exit() or returns from main, the whole process terminates (all threads).

	If it only exits its own thread (e.g., pthread\_exit() or syscall(SYS\_exit)), other threads keep running.

(3) How many underscores will be printed by the following program?

```c

#include <stdio.h>

#include <stddef.h>

#include <sys/types.h>

#include <unistd.h>

#include <sys/wait.h>



int main(){

      int i;

      for(i = 0; i<2; i++){

              fork();

              printf("\_\\n");

      }

      wait(NULL);

      wait(NULL);

      return 0;

}

```

Ans: 

(1) fork.c. 

(2) clone_share_global.c. Set flag = SIGCHLD. Like fork, They are different process and use different address space. They will use same variable. If parent destory, child operating as usually. Set flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD. They are the same address and process. They use different variable. If parent died, the child process will terminate immidately. 

(3) 6.

