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
* 2 (process name), 3 (state), 22 (starttime), 14 (user time) + 15 (system time), 23 (virtual mem size)
* Fields in /proc/pid/smaps we need for 1.2.2
* Rss (resident memory), Shared_Clean + Shared Dirty (shared memory)
*/
