# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/Altera/Documentos/face_recog

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/Altera/Documentos/face_recog

# Include any dependencies generated for this target.
include CMakeFiles/apc.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/apc.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/apc.dir/flags.make

CMakeFiles/apc.dir/apc.cpp.o: CMakeFiles/apc.dir/flags.make
CMakeFiles/apc.dir/apc.cpp.o: apc.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/Altera/Documentos/face_recog/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/apc.dir/apc.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/apc.dir/apc.cpp.o -c /home/Altera/Documentos/face_recog/apc.cpp

CMakeFiles/apc.dir/apc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/apc.dir/apc.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/Altera/Documentos/face_recog/apc.cpp > CMakeFiles/apc.dir/apc.cpp.i

CMakeFiles/apc.dir/apc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/apc.dir/apc.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/Altera/Documentos/face_recog/apc.cpp -o CMakeFiles/apc.dir/apc.cpp.s

CMakeFiles/apc.dir/apc.cpp.o.requires:
.PHONY : CMakeFiles/apc.dir/apc.cpp.o.requires

CMakeFiles/apc.dir/apc.cpp.o.provides: CMakeFiles/apc.dir/apc.cpp.o.requires
	$(MAKE) -f CMakeFiles/apc.dir/build.make CMakeFiles/apc.dir/apc.cpp.o.provides.build
.PHONY : CMakeFiles/apc.dir/apc.cpp.o.provides

CMakeFiles/apc.dir/apc.cpp.o.provides.build: CMakeFiles/apc.dir/apc.cpp.o

# Object files for target apc
apc_OBJECTS = \
"CMakeFiles/apc.dir/apc.cpp.o"

# External object files for target apc
apc_EXTERNAL_OBJECTS =

libapc.a: CMakeFiles/apc.dir/apc.cpp.o
libapc.a: CMakeFiles/apc.dir/build.make
libapc.a: CMakeFiles/apc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library libapc.a"
	$(CMAKE_COMMAND) -P CMakeFiles/apc.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/apc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/apc.dir/build: libapc.a
.PHONY : CMakeFiles/apc.dir/build

CMakeFiles/apc.dir/requires: CMakeFiles/apc.dir/apc.cpp.o.requires
.PHONY : CMakeFiles/apc.dir/requires

CMakeFiles/apc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/apc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/apc.dir/clean

CMakeFiles/apc.dir/depend:
	cd /home/Altera/Documentos/face_recog && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/Altera/Documentos/face_recog /home/Altera/Documentos/face_recog /home/Altera/Documentos/face_recog /home/Altera/Documentos/face_recog /home/Altera/Documentos/face_recog/CMakeFiles/apc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/apc.dir/depend
