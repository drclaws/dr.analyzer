# Dr. Analyzer
This software allows you to analyze your program to know which files and libraries it uses in real time. The using method is detouring selected windows api methods of your program's process.

The analyzer is for 64-bit only including your analyzed program but in future it will support 32-bit systems and programs.

Also it only supports single-process programs for now.

## Requirements
  + Windows 7 (x64) or higher
  + .Net Framework 4.6.2
  + User with Admin Rights

## Building

### Tools
  + MSBuild
  + .Net Framework 4.6.2 SDK
  + MSVC build tools x64/x86 v142
  + Windows SDK
  
### Steps
  1) Clone the repo with submodules
  2) Build solution using MSBuild
