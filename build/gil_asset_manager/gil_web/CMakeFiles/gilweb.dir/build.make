# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gilqamesh/Projects/league-tracker

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gilqamesh/Projects/league-tracker/build

# Include any dependencies generated for this target.
include gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/compiler_depend.make

# Include the progress variables for this target.
include gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/progress.make

# Include the compile flags for this target's objects.
include gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/flags.make

gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.o: gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/flags.make
gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.o: /home/gilqamesh/Projects/league-tracker/gil_asset_manager/gil_web/web.cpp
gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.o: gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/gilqamesh/Projects/league-tracker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.o"
	cd /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.o -MF CMakeFiles/gilweb.dir/web.cpp.o.d -o CMakeFiles/gilweb.dir/web.cpp.o -c /home/gilqamesh/Projects/league-tracker/gil_asset_manager/gil_web/web.cpp

gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/gilweb.dir/web.cpp.i"
	cd /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gilqamesh/Projects/league-tracker/gil_asset_manager/gil_web/web.cpp > CMakeFiles/gilweb.dir/web.cpp.i

gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/gilweb.dir/web.cpp.s"
	cd /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gilqamesh/Projects/league-tracker/gil_asset_manager/gil_web/web.cpp -o CMakeFiles/gilweb.dir/web.cpp.s

# Object files for target gilweb
gilweb_OBJECTS = \
"CMakeFiles/gilweb.dir/web.cpp.o"

# External object files for target gilweb
gilweb_EXTERNAL_OBJECTS =

gil_asset_manager/gil_web/libgilweb.a: gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/web.cpp.o
gil_asset_manager/gil_web/libgilweb.a: gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/build.make
gil_asset_manager/gil_web/libgilweb.a: gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/gilqamesh/Projects/league-tracker/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libgilweb.a"
	cd /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web && $(CMAKE_COMMAND) -P CMakeFiles/gilweb.dir/cmake_clean_target.cmake
	cd /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gilweb.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/build: gil_asset_manager/gil_web/libgilweb.a
.PHONY : gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/build

gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/clean:
	cd /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web && $(CMAKE_COMMAND) -P CMakeFiles/gilweb.dir/cmake_clean.cmake
.PHONY : gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/clean

gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/depend:
	cd /home/gilqamesh/Projects/league-tracker/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gilqamesh/Projects/league-tracker /home/gilqamesh/Projects/league-tracker/gil_asset_manager/gil_web /home/gilqamesh/Projects/league-tracker/build /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web /home/gilqamesh/Projects/league-tracker/build/gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : gil_asset_manager/gil_web/CMakeFiles/gilweb.dir/depend

