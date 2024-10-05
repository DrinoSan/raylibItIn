cmake_minimum_required(VERSION 3.10)

# Set your project name
project(Game)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Use pkg-config to find Raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# Find GLFW using CMake's find_package
find_package(glfw3 REQUIRED)

# Add executable
add_executable(main main.c)

# Include directories
target_include_directories(main PRIVATE ${RAYLIB_INCLUDE_DIRS})

# Link directories (explicitly add the library path)
target_link_directories(main PRIVATE ${RAYLIB_LIBRARY_DIRS})

# Link libraries
target_link_libraries(main
    ${RAYLIB_LIBRARIES}   # This should resolve to -lraylib
    glfw
    -lm
    -lpthread
    "-framework CoreFoundation"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework OpenGL"
)

#Add ctags generation (optional)
find_program(CTAGS_EXECUTABLE ctags)
if(CTAGS_EXECUTABLE)
    add_custom_target(tags
        COMMAND ${CTAGS_EXECUTABLE} -R .
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating ctags"
    )
endif()
