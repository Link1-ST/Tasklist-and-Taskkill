/* Author: Saarthik Tannan
   Purpose: Emulate Taskkill options: /PID, /IM, and /T
   Version: 1.0 */

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <iostream>
#include <string>

#pragma warning(disable:4996)


int PID() {
    // Purpose: Terminates the process by its PID
    
    int processID;
    HANDLE hProcess;
    
    // User enters the process ID for process to kill
    printf("Enter process ID: \n");
    scanf("%d", &processID);

    // Check that user entered PID
    if (processID == NULL)
    {
        printf("Process ID is required to kill\n");
        return -1;
    }

    // Check if process ID is negative and return -1
    if (processID < 0)
    {
        printf("Process ID should be a positive integer\n");
        return -1;
    }

    // Open handle to process
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);

    // Check if hProcess is NULL, and print message 
    if (hProcess == NULL)
    {
        printf( "Cannot open process with ID %d\n", processID);
        return -1;
    }

    // Terminate process
    if (TerminateProcess(hProcess, -1))
    {
        printf("Process with ID %d has been terminated\n", processID);
    }

    // Print message if error with termination
    else
    {
        printf("Termination of the process with ID %d failed\n", processID);
        CloseHandle(hProcess);
        return -1;
    }

    // Close the handle to the process
    CloseHandle(hProcess);

    return 0;
}

int IM() {
    // Purpose: Terminate the process by its name

    char processName [1024] = "";
    std::string processNameStr = "";
    std::string szProcNameStr = "";
    HANDLE hProcess = NULL;
    DWORD processes[1024], cbNeeded, count, cProcesses;
    HMODULE hMod;
    unsigned int i = 0;

    // User enters image name for process to kill
    printf("Enter process name (i.e. something.exe):\n");
    scanf("%s", &processName);
    // Save the value of processName into processNameStr
    processNameStr = (std::string)processName;


    // Get all the process so that the process ID can be obtained
    if (!EnumProcesses(processes, sizeof(processes), &count))
    {
        printf("Cannot kill process %s\n", processName);
        return -1;
    }

    // Calculate how many process identifiers were returned
    count = count / sizeof(DWORD);

    // Loop through the processes
    for (i = 0; i < count; i++)
    {

        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

        if (processes[i] != 0) {
            // Create the handle to process
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[i]);

            // Handle is successfully created 
            if (hProcess != NULL)
            {
                // Enumarate processes
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                {
                    GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));

                    // Convert szProcessName to std::string
                    std::wstring ws(szProcessName);
                    std::string szProcNameStr(ws.begin(), ws.end());

                    // Find the process and kill it
                    if (processNameStr.compare(szProcNameStr) == 0)
                    {
                        DWORD result = WAIT_OBJECT_0;

                        while (result == WAIT_OBJECT_0)
                        {
                            // Use WaitForSingleObject to make sure the process is dead
                            result = WaitForSingleObject(hProcess, 100);
                            TerminateProcess(hProcess, 0);
                            printf("Process %s killed\n", processName);
                        }

                        // Release the handle to the process
                        CloseHandle(hProcess);
                        return 0;
                    }

                }
            }
        }

    }
    
    // Print message if process cannot be found
    if (!(processNameStr.compare(szProcNameStr) == 0)) 
    {
        printf("Process %s cannot be found\n", processName);
        CloseHandle(hProcess);
        return -1;
    }
}

int T() {
    /* Purpose: Tree kill: terminates the specified process
    and any child processes which were started by it */

    int processID;

    PROCESSENTRY32 pe;

    // User enters the process ID for process to kill
    printf("Enter process ID: \n");
    scanf("%d", &processID);

    // Check that user entered PID
    if (processID == NULL)
    {
        printf("Process ID is required to kill\n");
        return -1;
    }

    // Check if process ID is negative and return -1
    if (processID < 0)
    {
        printf("Process ID should be a positive integer\n");
        return -1;
    }

    // Set the memory
    memset(&pe, 0, sizeof(PROCESSENTRY32));
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    // Call CreateToolhelp32Snapshot to enumerate all the processes on the system
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    // Check first process
    if (Process32First(hSnap, &pe))
    {
        BOOL bContinue = TRUE;

        // Kill child processes if they exist
        while (bContinue)
        {
            // Only kill child processes
            if (pe.th32ParentProcessID == processID)
            {
                // Create a handle to the child process
                HANDLE hChildProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);

                if (hChildProc)
                {
                    TerminateProcess(hChildProc, 1);
                    printf("Child process with ID %d killed\n", pe.th32ProcessID);
                    CloseHandle(hChildProc);
                }
                
            }

            // Check the next process
            bContinue = Process32Next(hSnap, &pe);
        }

        // Kill the main process if it exists
        HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

        if (hProc)
        {
            TerminateProcess(hProc, 1);
            printf("Parent process with ID %d killed\n", processID);
            // Close the handle to the process
            CloseHandle(hProc);
            return 0;
        }

        else 
        {
            printf("Process with ID %d cannot be found\n");
            // Close the handle to the process
            CloseHandle(hProc);
            return -1;

        }
        
    }
}

int main() {
    // Store the option 
    int option = 1;

    // Run the loop until the user enters 0
    while (option != 0)
    {
        // Select choice for tasklist option
        printf("Tasklist Options:\n");
        printf("Enter 0 to exit\n");
        printf("Enter 1 for /PID (terminates process by its PID)\n");
        printf("Enter 2 for /IM (terminates process by its image name)\n");
        printf("Enter 3 for /T (terminates the specified process and any child processes which were started by it.)\n");


        printf("Enter option:\n");
        scanf("%d", &option);
        printf("\n");

        switch (option)
        {

        case 0:
            exit(0);

        case 1:
            PID();
            printf("\n");
            break;

        case 2:
            IM();
            printf("\n");
            break;

        case 3:
            T();
            printf("\n");
            break;

        default:
            printf("That is not a valid option!\n");
            printf("\n");
            break;

        }


    }
}