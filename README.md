# Process Reimaging

This repo contains proof-of-concept C++/C# code to perform the process reimaging technique described in:

https://securingtomorrow.mcafee.com/other-blogs/mcafee-labs/in-ntdll-i-trust-process-reimaging-and-endpoint-security-solution-bypass/

This allows you to launch one executable, say bad.exe, and have process enumeration techniques point to another executable. This is due to the stale FILE_OBJECTs the kernel32 APIs use.

## Usage

```
.\poc.exe C:\path\to\bad.exe C:\path\to\good.exe```