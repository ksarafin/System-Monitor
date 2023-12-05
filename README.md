# CS252 Project 6

## Team Members:
Katherine Sarafin  
Sreeya Yekollu  
Elise Kwan  

## Instructions:
The Makefile included will generate an executable titled 'system_monitor'. 
Run the Makefile and execute 'system_monitor' to view and grade our project.

## Technologies:
Our project was organized and updated using a private repository on GitHub. 
We used the C language and employed GTK to write our GUI. The layout of our files is as follows:  

* system_monitor.c contains the majority of the code to develop and run the GUI. It calls on functions 
from other files to generate the information needed for the GUI.  

* system_info.c generates information pertaining to section 1.1 in the handout. 

* process_table.c generates information pertaining to section 1.2 in the handout. 

* ui.c, util.c, and resource_graph.c generate graphs and information pertaining to section 1.3 in the handout

* file_system_table generates information pertaining to section 1.4 in the handout. 

* All .h files are header files with function and variable declarations for clarity.

## Features:
Our system monitor contains four tabs, one for each section of the handout as detailed above. 
  
The first tab is called 'System' and displays basic system information.
  
The second tab is called 'Processes' and displays information about running processes. The process list is updated at a fixed interval of 5 seconds. Above the four tabs is a 'View' menu that can be used to switch between viewing all processes, only processes owned by the user, or active processes (extra feature). Next to the 'View' menu is the 'File' menu which exits the window. Right clicking on any process displays a menu with options to stop, continue, or kill the process as well as options to view memory maps and open files. Double clicking on any process opens a new window displaying further details about the process. An extra feature we added is the load averages above the processes table. 
  
The third tab is called 'Resources' and displays graphs for cpu usage, memory usage, and network usage. These graphs update at a fixed interval of 1000 milliseconds.    
  
The fourth tab is called 'File Systems' and displays a table of information regarding the usage of each mount point. 

## Extra features: 
* View Active Processes
* Load Averages in Processes tab

