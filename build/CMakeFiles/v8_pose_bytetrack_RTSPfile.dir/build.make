# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

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
CMAKE_COMMAND = /root/.local/bin/cmake

# The command to remove a file.
RM = /root/.local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /app/Robin/v8_pose_bytetrack_file

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /app/Robin/v8_pose_bytetrack_file/build

# Include any dependencies generated for this target.
include CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/flags.make

CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.o: CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/flags.make
CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.o: ../v8_pose_bytetrack_RTSPfile.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/app/Robin/v8_pose_bytetrack_file/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.o -c /app/Robin/v8_pose_bytetrack_file/v8_pose_bytetrack_RTSPfile.cpp

CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /app/Robin/v8_pose_bytetrack_file/v8_pose_bytetrack_RTSPfile.cpp > CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.i

CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /app/Robin/v8_pose_bytetrack_file/v8_pose_bytetrack_RTSPfile.cpp -o CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.s

# Object files for target v8_pose_bytetrack_RTSPfile
v8_pose_bytetrack_RTSPfile_OBJECTS = \
"CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.o"

# External object files for target v8_pose_bytetrack_RTSPfile
v8_pose_bytetrack_RTSPfile_EXTERNAL_OBJECTS =

v8_pose_bytetrack_RTSPfile: CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/v8_pose_bytetrack_RTSPfile.cpp.o
v8_pose_bytetrack_RTSPfile: CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/build.make
v8_pose_bytetrack_RTSPfile: /usr/lib/x86_64-linux-gnu/libnvinfer.so
v8_pose_bytetrack_RTSPfile: CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/app/Robin/v8_pose_bytetrack_file/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable v8_pose_bytetrack_RTSPfile"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/build: v8_pose_bytetrack_RTSPfile

.PHONY : CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/build

CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/cmake_clean.cmake
.PHONY : CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/clean

CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/depend:
	cd /app/Robin/v8_pose_bytetrack_file/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /app/Robin/v8_pose_bytetrack_file /app/Robin/v8_pose_bytetrack_file /app/Robin/v8_pose_bytetrack_file/build /app/Robin/v8_pose_bytetrack_file/build /app/Robin/v8_pose_bytetrack_file/build/CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/v8_pose_bytetrack_RTSPfile.dir/depend
