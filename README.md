# Tasklist and Taskkill

Author: Saarthik Tannan

Version: 1.0


Instructions for using tasklist and taskkill programs:
```
1. If the inclued exe files do not work then compile it with Visual Studio or any other compiler
2. Run tasklist-v1.0.exe or taskill-v1.0.exe
3. Enter the option choice (0, 1, 2, or 3)

NOTE:
1. These tools only work locally.  They prompt users for input (i.e. 0, 1, 2, and 3)
3. Some process information may not be found and returns Unknown.  
4. Some process information may be found if running with admin priviledges.
5. Some process names may not be found and cannot be killed by it image name.
```
Tasklist options:
```
0. Exit
1. /S returns all the running processes
2. /SVC returns information for each process without truncation 
3. /V returns verbose task information
```
Taskkill options:
```
0. Exit
1. /PID terminates process by its PID
2. /IM  terminates process by its image name
3. /T terminates the specified process and any child processes which were started by it
```
