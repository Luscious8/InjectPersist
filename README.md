# InjectPersist

C++ stager that downloads shellcode and injects into explorer.exe for Windows persistence. Bypasses Windows Defender. Keeps C2 callback alive after terminal closes.

## Features

- Downloads raw shellcode over HTTP
- Injects directly into explorer.exe (system process)
- No malicious file on disk after execution
- Bypasses Windows Defender (tested on Windows 11)
- Shell survives after stager.exe exits

## How It Works

1. Stager downloads shellcode from your C2 server
2. Finds explorer.exe PID using Toolhelp32Snapshot
3. Opens explorer.exe with PROCESS_ALL_ACCESS
4. Allocates RWX memory in explorer.exe
5. Writes shellcode using WriteProcessMemory
6. Executes via CreateRemoteThread
7. Stager exits - shell lives in explorer.exe

## Requirements

- Kali Linux (for compilation) or any Linux with MinGW
- Windows 10/11 target
- Any C2 that generates raw x64 shellcode (Adaptix, Cobalt Strike, Sliver, MSFVenom)

## Compilation (on Kali)

```bash
# Install MinGW if not installed
sudo apt install mingw-w64

# Compile stager
x86_64-w64-mingw32-g++ -static -Os -s -o stager.exe stager.cpp -lwininet -mwindows
