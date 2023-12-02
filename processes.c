/* 
* So this only prints out all the info we need, barring memory, to the terminal.
* I'll probably adjust it to only print pid and command name for the initial menu and then run
* execv again and grep by pid for the extra information section. Just wanted to get the fork and
* redirection working before I complicated things. 
*/        

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
        // Initialize pipes and other stuff
        int pipes[2];
        int ret;
        char buffer[4096];

        pipe(pipes);
        ret = fork();

        if (ret == 0) {
          // Redirect output and call exec on "ps -eo pid,cmd,lstart,s,time,%cpu"
          dup2(pipes[1], STDOUT_FILENO);
          close(pipes[1]);
          close(pipes[0]);
            
          // Pid used to grep for info, cmd give process name, lstart gives start time and date,
          // s gives state, others are self explanatory
          char * args[4] = {"ps", "-eo", "pid,cmd,lstart,s,time,%cpu", NULL};
          execv("/bin/ps", args);
        }
        else {
          close(pipes[1]);

          // Read input from pipe and print to screen to check that it's correct     
          while (1) {
            int nbytes = read(pipes[0], buffer, sizeof(buffer));

            // If nothing read, print new line  
            if (nbytes == 0) {
              printf("\n");
              break;
            }

            printf("%s", buffer);
          }
        }
}
}
