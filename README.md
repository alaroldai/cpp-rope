cproj_template
==============

Template for C/C++ projects.

Sets up a build system and YouCompleteMe configuration for a new C/C++ project.

## Build system
The build system is driven by `make`, which is responsible for creating directories for sub-build systems if needed. Otherwise, it just hands off to `CMake`, which is where the bulk of the configuration should take place.

Two sub-build systems (Ninja and XCode) are already provided for. Adding others should be fairly trivial - take a look at the Makefile for examples.

## YouCompleteMe support
YouCompleteMe is supported through a .ycm_extra_conf.py file that loads up a compile commands database. The database is refreshed each time you build the project.
