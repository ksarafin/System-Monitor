system_monitor: system_monitor.c util.c ui.c system_info.c process_table.c file_system_table.c resource_graph.c
        gcc -o system_monitor $(shell pkg-config --cflags gtk+-3.0) system_monitor.c util.c ui.c system_info.c process_table.c file_system_table.c resource_graph.c $(shell pkg-config --libs gtk+-3.0)
