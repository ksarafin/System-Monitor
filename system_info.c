#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "system_info.h"

unsigned long get_available_disk_space() {
    FILE *fp = popen("df / --output=avail | tail -n 1", "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    unsigned long available_space;
    fscanf(fp, "%lu", &available_space);

    // Close the pipe
    pclose(fp);

    return available_space;
}

void get_system_info(char* info_buffer, int buffer_size) {
    struct utsname system_info;
    uname(&system_info);

    struct sysinfo info;
    sysinfo(&info);

    unsigned long available_space = get_available_disk_space();

    snprintf(info_buffer, buffer_size,
             "<b>System Information</b>\n\n"
             "<b>OS Release:</b> %s\n"
             "<b>Kernel Version:</b> %s\n"
             "<b>Memory:</b> %lu MB\n"
             "<b>Processor:</b> %s\n"
             "<b>Available Disk Space:</b> %lu MB",
             system_info.release,
             system_info.version,
             (info.totalram / 1024) / 1024,
             system_info.machine,
             available_space / 1024);
}

void get_cpu_info(char* info_buffer, int buffer_size) {
    FILE *fp = popen("lscpu", "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    // Read the output of the command
    fgets(info_buffer, buffer_size, fp);

    // Close the pipe
    pclose(fp);
}

