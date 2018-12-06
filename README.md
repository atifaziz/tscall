# `tscall`

`tscall` is a small utility for Windows for invoking a time-stamped copy of a
batch script. The Windows Command Processor (`cmd.exe`) does not buffer a
batch script prior to execution. Updating a batch script while it is executing
has the effect of corrupting its execution. `tscall` solves the problem by
making a copy of a batch script based on its last modified timestamp and then
executing that copy.


## Usage

    tscall BATFILE [ARGS...]

where `BATFILE` is the file path of the batch script to be time-stamped and
executed. All tail arguments (`ARGS...`), if any, are passed on to the batch
script.

The time-stamped script is created as a hidden file in the same directory as
the original. The file name is modified to have a timestamp suffix in the
form of `-%Y%m%d%H%M%S`. If such a file already exists (presumably from a
previous execution) then it is used as-is.


## Building

Building requires an installation of the Microsoft Visual C++ compiler. Only
the version

Run `build.cmd` found in the root of the code repository.
