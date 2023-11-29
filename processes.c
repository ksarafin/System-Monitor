/* 
* CS252-Proj6
*
* Basic Code for getting stuff from /proc:
* https://stackoverflow.com/questions/63372288/getting-list-of-pids-from-proc-in-linux
* 
* Fields in /proc/pid/stat we need for 1.2
* 1 (pid), 2 (process name)
*
* Fields in /proc/pid/stat we need for 1.2.2
* 2 (process name), 3 (state), 14 (user time) + 15 (system time), 23 (virtual mem size)
* Fields in /proc/pid/smaps we need for 1.2.2
* Rss (resident memory), Shared_Clean + Shared Dirty (shared memory)
* 
* Grep for pid from 'ps -eo pid,lstart' for start date/time
*
* Don't know how to get total memory (sum all sections?) or cpu time (user + sys?)
*/

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>

// Helper function to check if a struct dirent from /proc is a PID directory.
int is_pid_dir(const struct dirent *entry) {
    const char *p;

    for (p = entry->d_name; *p; p++) {
        if (!isdigit(*p))
            return 0;
    }

    return 1;
}

int main(void) {
    DIR *procdir;
    FILE *fp;
    struct dirent *entry;
    char path[256 + 5 + 5]; // d_name + /proc + /stat
    
    int pid;
    char proc_name[256];
    char state:
    unsigned long sys_time;
    unsigned long user_time;
    unsigned long vm_size;
    

    // Open /proc directory.
    procdir = opendir("/proc");
    if (!procdir) {
        perror("opendir failed");
        return 1;
    }

    // Iterate through all files and directories of /proc.
    while ((entry = readdir(procdir))) {
        // Skip anything that is not a PID directory.
        if (!is_pid_dir(entry))
            continue;

        // Try to open /proc/<PID>/stat.
        snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
        fp = fopen(path, "r");

        if (!fp) {
            perror(path);
            continue;
        }

        // Get PID, process name and number of faults.
        fscanf(fp, "%d %s %c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %lu",
            &pid, &proc_name, &state, &user_time, %sys_time, &vm_size); 
        );

        // Pretty print.
        printf("%5d %-20s: %c %lu %lu\n", pid, path, state, user_time + sys_time, vm_size);
        fclose(fp);
    }

    closedir(procdir);
    return 0;
}
