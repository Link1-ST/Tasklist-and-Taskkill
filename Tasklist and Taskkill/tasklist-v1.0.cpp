/* Author: Saarthik Tannan
   Purpose: Emulate Tasklist options: /S, /V, and /SVC.  
   Include some additional information not provided with the native Windows Tasklist.
   Version: 1.0 */

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <processthreadsapi.h>
#include <vector>
#include <iostream>
#include <ATLComTime.h>


#define MAX_NAME 256

#pragma warning(disable:4996)

void PrintProcessNameAndID(DWORD processID) {
    // Purpose: Print the process name and ID number

    TCHAR szProcessName[MAX_PATH] = TEXT("<Unknown>");

    // Get a handle to the process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);

    // Get the process name
    if (hProcess != NULL) 
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
            &cbNeeded)) 
        {
            GetModuleBaseName(hProcess, hMod, szProcessName,
                sizeof(szProcessName) / sizeof(TCHAR));
        }
    }

    // Print the process name and identifier
    _tprintf(TEXT("Process Name and ID: %s  (PID: %u)\n"), szProcessName, processID);

    // Release the handle to the process
    CloseHandle(hProcess);
}

void PrintModules(DWORD processID)
{
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    // Get a handle to the process
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);
    
    // Check if hProcess is NULL
    if (hProcess == NULL)
    {
        printf("Modules: Unknown\n");
        return;
    }

    // Get a list of all the modules in this process
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        printf("Modules: \n");
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];

            // Get the full path to the module's file
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
                sizeof(szModName) / sizeof(TCHAR)))
            {
                // Print the module name and handle value
                _tprintf(TEXT("\t%s (%0x%08X))\n"), szModName, hMods[i]);
            }
        }
    }

    // Release the handle to the process
    CloseHandle(hProcess);
}

void PrintSessionID(DWORD processID) {
    // Purpose: Print the Session name and ID

    DWORD SessionID = 0;
    HANDLE hProcess;

    // Get a handle to the process
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);
    
    // Get the Session ID
    if (hProcess != NULL)
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
            &cbNeeded))
        {
            ProcessIdToSessionId(processID, &SessionID);
        }
    }

    // Print the Session ID
    printf("Session ID: %ld\n", SessionID); 
 
    // Release the handle to the process
    CloseHandle(hProcess);
}

void PrintMemoryInfo(DWORD processID) {
    // Purpose: Print the memory usage information for a process 

    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    // Get a handle to the process
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);

    // Check if hProcess is NULL
    if (hProcess == NULL)
    {
        printf("Memory Usage: Unknown\n");
        return;
    }

    // Print the memory usage information
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) 
    {
        printf("Memory Usage: \n");
        printf("\tPageFaultCount: %d\n", (int)pmc.PageFaultCount);
        printf("\tPeakWorkingSetSize: %d\n",
            (int)pmc.PeakWorkingSetSize);
        printf("\tWorkingSetSize: %d\n", (int)pmc.WorkingSetSize);
        printf("\tQuotaPeakPagedPoolUsage: %d\n",
            (int)pmc.QuotaPeakPagedPoolUsage);
        printf("\tQuotaPagedPoolUsage: %d\n",
            (int)pmc.QuotaPagedPoolUsage);
        printf("\tQuotaPeakNonPagedPoolUsage: %d\n",
            (int)pmc.QuotaPeakNonPagedPoolUsage);
        printf("\tQuotaNonPagedPoolUsage: %d\n",
            (int)pmc.QuotaNonPagedPoolUsage);
        printf("\tPagefileUsage: %d\n", (int)pmc.PagefileUsage);
        printf("\tPeakPagefileUsage: %d\n",
            (int)pmc.PeakPagefileUsage);
    }
    
    // Close the handle to the process
    CloseHandle(hProcess);
}

void PrintServiceName(DWORD processID) {
    // Purpose: Print the service name

    // aPID will be used later for check if serive name cannot be obtained
    DWORD aPID = 0;

    // Get a handle to the service
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL,
        SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);

    // Check if hSCM is NULL
    if (hSCM == NULL)
    {
        printf("Service: Unknown\n");
        CloseServiceHandle(hSCM);
        return;
    }

    DWORD bufferSize = 0;
    DWORD requiredBufferSize = 0;
    DWORD totalServicesCount = 0;

    EnumServicesStatusEx(hSCM,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        nullptr,
        bufferSize,
        &requiredBufferSize,
        &totalServicesCount,
        nullptr,
        nullptr);

    // Set the buffer 
    std::vector<BYTE> buffer(requiredBufferSize);
    EnumServicesStatusEx(hSCM,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        buffer.data(),
        buffer.size(),
        &requiredBufferSize,
        &totalServicesCount,
        nullptr,
        nullptr);

    // Get the service name.  Print message is service name cannot be obtained
    LPENUM_SERVICE_STATUS_PROCESS services =
        reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(buffer.data());
    ENUM_SERVICE_STATUS_PROCESS service;
    for (unsigned int i = 0; i < totalServicesCount; ++i)
    {
        service = services[i];
        if (service.ServiceStatusProcess.dwProcessId == processID)
        {
            // Print the service name
            _tprintf(TEXT("Service: %Ls\n"), service.lpServiceName);
            
            // Set aPID to service.ServiceStatusProcess.dwProcessId
            aPID = service.ServiceStatusProcess.dwProcessId;
        }

    }

    // Print message if service name cannot be obtained
    if (aPID == 0)
    {
        printf("Service: Unknown\n");
    }

    // Release the handle to the service
    CloseServiceHandle(hSCM);
}

void PrintStatus(DWORD processID) {
    // Purpose: Print the status of the process

    HANDLE hProcess;
    DWORD ret;

    // Get a handle to the process
    hProcess = OpenProcess(SYNCHRONIZE, FALSE, processID);

    ret = WaitForSingleObject(hProcess, 0);

    // Check if ret == WAIT_TIMEOUT; the value of WAIT_TIMEOUT is 259 which specifies that the process is running
    if (ret == WAIT_TIMEOUT) {
        printf("Status: Running\n");
    }
    
    else {
        printf("Status: Unknown\n");
    }

    // Release the handle to the process
    CloseHandle(hProcess);
}

void PrintUsername(DWORD processID) {
    // Purpose: Print the username associated with the process

    HANDLE hProcess;
    HANDLE hToken;
    DWORD dwSize = MAX_NAME;
    DWORD dwLength = 0;
    PTOKEN_USER ptu = NULL;

    // Get a handle to the process
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);

    // Verify that the value of hProcess is not NULL
    if (hProcess == NULL) {
        printf("Username: Unknown\n");
        return;
    }

    // Get a handle to the access token for the process
    OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);

    // Verify the parameter passed in is not NULL
    if (hToken == NULL) 
    {
        goto Cleanup;
    }

    // Check if there is a problem with the GetTokenInformation function
    if (!GetTokenInformation(
        hToken,         // handle to the access token
        TokenUser,      // get information about the token's groups 
        (LPVOID)ptu,    // pointer to PTOKEN_USER buffer
        0,              // size of buffer
        &dwLength       // receives required buffer size
    ))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Cleanup;
        }
        ptu = (PTOKEN_USER)HeapAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, dwLength);

        if (ptu == NULL)
        {
            goto Cleanup;
        }
    }
    
    // Check if there is a problem with the GetTokenInformation function
    if (!GetTokenInformation(
        hToken,         // handle to the access token
        TokenUser,      // get information about the token's groups 
        (LPVOID)ptu,    // pointer to PTOKEN_USER buffer
        dwLength,       // size of buffer
        &dwLength       // receives required buffer size
    ))
    {
        goto Cleanup;
    }

    SID_NAME_USE SidType;
    char lpName[MAX_NAME];
    char lpDomain[MAX_NAME];

    // Print the username
    if (LookupAccountSid(NULL, ptu->User.Sid, (LPWSTR)lpName, &dwSize, (LPWSTR)lpDomain, &dwSize, &SidType))
    {
        _tprintf(TEXT("Username: %s\\%s\n"), lpDomain, lpName);
    }

Cleanup:

    if (ptu != NULL)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)ptu);
    }
}

void PrintProcessTime(DWORD processID) {
    // Purpose: Print the process time (creation time, running time, kernel time, and user time)

    HANDLE hProcess;
    FILETIME ftCreation,
        ftExit,
        ftKernel,
        ftUser;

    // Get a handle to the process
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);

    // Call GetProcessTimes
    GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser);

   // Calculate the running time
    COleDateTime timeNow = COleDateTime::GetCurrentTime(),
        timeCreation = ftCreation;
    COleDateTimeSpan timeDiff = timeNow - timeCreation;
    
    // Get the kernel time
    SYSTEMTIME stKernel;
    FileTimeToSystemTime(&ftKernel, &stKernel);

    // Get the user time
    SYSTEMTIME stUser;
    FileTimeToSystemTime(&ftUser, &stUser);

    // Call GetProcessTimes to set the values 
    GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser);
    timeCreation = ftCreation;
    
    // Print the creation time, running time, kernel time, and user time
    printf("Created Time: %02d:%02d:%02d\n", timeCreation.GetHour(),
        timeCreation.GetMinute(), timeCreation.GetSecond());
    printf("Running Time: %ud %uh %um %us\n", timeDiff.GetDays(),
        timeDiff.GetHours(), timeDiff.GetMinutes(),
        timeDiff.GetSeconds());
    printf("Kernel Time: %uh %um %us\n", stKernel.wHour,
        stKernel.wMinute, stKernel.wSecond);
    printf("User Time: %uh %um %us\n", stUser.wHour,
        stUser.wMinute, stUser.wSecond);

    // Release the handle to the process
    CloseHandle(hProcess);
}

int S() {
	// Purpose: Tasklist /S option returns the running processes
    
    // Get the list of process identifiers
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) 
    {
        return 1;
    }

    // Calculate how many process identifiers were returned
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the process name and ID, session ID, and memory usage
    for (i = 0; i < cProcesses; i++) 
    {
        if (aProcesses[i] != 0) 
        {
            PrintProcessNameAndID(aProcesses[i]);
            PrintSessionID(aProcesses[i]);
            PrintMemoryInfo(aProcesses[i]);
            printf("\n");
        }
    }

    return 0;
}

int SVC() {
    // Purpose: Tasklist /SVC option returns the information for each process without truncation (i.e. Process name and ID, and services)

    // Get the list of process identifiers
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 1;
    }

    // Calculate how many process identifiers were returned
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the process name and ID, and the services
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            PrintProcessNameAndID(aProcesses[i]);
            PrintServiceName(aProcesses[i]);
            printf("\n");
        }
    }

    return 0;
}

int V() {
    // Purpose: Tasklist /V option displays verbose task information in the output 

    // Get the list of process identifiers
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 1;
    }

    // Calculate how many process identifiers were returned
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the process name and ID, session ID, memory usage, status, username, process time (creation time, running time, kernel time, and user time), and modules
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            PrintProcessNameAndID(aProcesses[i]);
            PrintSessionID(aProcesses[i]);
            PrintMemoryInfo(aProcesses[i]);
            PrintStatus(aProcesses[i]);
            PrintUsername(aProcesses[i]);
            PrintProcessTime(aProcesses[i]);
            PrintModules(aProcesses[i]);
            printf("\n");
        }

    }

    return 0;
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
        printf("Enter 1 for /S (returns all the running processes)\n");
        printf("Enter 2 for /SVC (returns information for each process without truncation)\n");
        printf("Enter 3 for /V (returns verbose task information)\n");
        
        printf("Enter option:\n");
        scanf("%d", &option);
        printf("\n");

        switch (option) 
        {

        case 0:
            exit(0);

        case 1:
            S();
            printf("\n");
            break;
        
        case 2:
            SVC();
            printf("\n");
            break;
        
        case 3:
            V();
            printf("\n");
            break;

        default:
            printf("That is not a valid option!\n");
            printf("\n");
            break;
            
        }
        

    }
    
}