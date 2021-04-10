# Tasklist and Taskkill

Author: Saarthik Tannan

Version: 1.0


Instructions for using tasklist and taskkill programs:
```
1. If the inclued exe files do not work then compile it with Visual Studio or any other compiler
2. Run tasklist-v1.0.exe or taskill-v1.0 (Note: Some processes may not be found or killed if not running with admin priviledges)
3. Enter the option choice (1, 2, or 3)
```
Tasklist options:
```
1. /S returns all the running processes
2. /SVC returns information for each process without truncation 
3. /V returns verbose task information
```
Taskkill options:
```
1. /PID terminates process by its PID
2. /IM  terminates process by its image name
3. /T terminates the specified process and any child processes which were started by it
```
