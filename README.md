# Dr. Analyzer
This software allows you to analyze your program to know which files and libraries it uses. The using method is detouring selected windows api calls of your program's process.

The analyzer is for 64-bit only including your analyzed program but in future it will support 32-bit systems and programs.

## Requirements

  + [Detours](https://github.com/microsoft/detours) – detouring library developed by Microsoft
  + [Handle](https://docs.microsoft.com/en-us/sysinternals/downloads/handle) (optionally) – used in project for getting list of opened files on analyze start
  + Visual C++ Build Toolchain 2017
  + .Net Framework 4.6.2

## Building and using

1) Clone this repository
2) Build Detours
3) Put "include" directory to "libraries/include" and 64-bit library to "libraries/x64"
4) Build solution with Visual Studio Tools
5) (Optionally) Put handle64.exe near Analyzer executable
