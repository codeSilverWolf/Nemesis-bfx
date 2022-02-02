# Project Nemesis-bfx

## What is Nemesis-bfx?

It's a somewhat bugfixed edition of [Nemesis Unlimited Behavior Engine's](https://github.com/ShikyoKira/Project-New-Reign---Nemesis-Main) *main program*, that was ported to GCC compiler and MSYS2 Software Distribution and Building Platform for Windows.
Code is based on latest code that was available on Nemesis Engine's github (commit de71cdcc9988f556dbff2385dc779482e774b74a October 7th, 2020. Newer than v0.84beta).

Why do this?
Nemesis Engine v0.84 beta is rather unstable. On my machine it can not even finish updating or generating behaviors without restricting it to single thread. And since i wanted to learn C++ Nemesis looked like a good practice target to try on.

## Main differences to original Nemesis
- Only main executable "Nemesis Unlimited Behavior Engine.exe" and required libraries are provided, you need to get rest of mod (behavior files scripts etc.) from Nemesis v0.84beta.
- focus on stability and bugfixes not performance. Multiple crash/bug fixes. Multithreading update/behavior generate is disabled for now.
- build using different c++ compiler and libraries.
- enabled message output to console when run from command prompt (cmd.exe or other console program). This can help debugging crashes that do not leave trace in log files.
- support for python library files in one zip archive (2k files less needed in mod)
- New bugs most likely introduced. Since Nemesis-bfx is based on newer unreleased code and generally Nemesis code looks like it is in middle of rather substantial changes. Building this with gcc for MSYS2 most likely introduced some subtle bugs. Backup your nemesis folder before overwriting with this.

## Project status

- Works for me :) . No binary release or detailed instructions how to compile yet (on my TODO soon list).
- does not work when Skyrim is installed in path which has non ASCII characters.
- Generally i hope ShikyoKira can find some of my fixes usefull and incorporate them in Nemesis Unlimited Behavior Engine.

## How to compile
1) Install [msys2](https://www.msys2.org/)
2) Install build tools and dependencies for MSYS2 gcc UCRT 64bit environment (aka UCRT64):
   - gcc compiler (mingw-w64-ucrt-x86_64-toolchain)
   - cmake (mingw-w64-ucrt-x86_64-cmake)
   - qt libraries
   - boost
   - python
3) Clone, configure and compile from msys2 UCRT 64bit console.
4) make mod archive using "utils/makemod.py" script (this script has to be executed from msys console and in project build directory).
5) Backup Nemesis_Engine folder inside your skyrim Data folder. Install created zip file in your mod manager allowing it to overwrite Nemesis
