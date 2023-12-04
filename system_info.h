#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

unsigned long get_available_disk_space();
void get_system_info(char* info_buffer, int buffer_size);
void get_cpu_info(char* info_buffer, int buffer_size);

#endif
