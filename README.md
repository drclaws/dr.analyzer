# Dr. Analyzer
This software allows you to analyze your program to know which files and libraries it uses. The using method is detouring selected windows api methods of your program's process.

The analyzer is for 64-bit only including your analyzed program but in future it will support 32-bit systems and programs.

Also it only supports single-process programs for now.

## Requirements

  + [Detours](https://github.com/microsoft/detours) – detouring library developed by Microsoft
  + MSBuild with support .Net Framework 4.6.2
  + MSVC build tools x64/x86 v141
  + Windows SDK

## Building

1) Clone [Detours](https://github.com/microsoft/detours) repository
2) Build the library
3) Clone this repository
3) Put "include" directory of Detours to "libraries/include" and x64 library to "libraries/x64"
4) Build solution with MSBuild
