cmake_minimum_required(VERSION 3.10)

project(IrisCLI VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default to Debug if not provided
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release)" FORCE)
endif()

# Define project source files
file(GLOB_RECURSE IRISCLI_SRC
  "${PROJECT_SOURCE_DIR}/src/*.c"
  "${PROJECT_SOURCE_DIR}/lib/**/*.c"
)

add_executable(iris ${IRISCLI_SRC})

# Make sure the project uses our include directory
target_include_directories(iris PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/lib")

# Compiler flags (add -Wall by default)
target_compile_options(iris PRIVATE -Wall)

# Place the runtime binary in the `build/` folder to match the existing Makefile
set_target_properties(iris PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build"
)

# Install target (optional)
install(TARGETS iris
  RUNTIME DESTINATION bin
)

message(STATUS "Configured IrisCLI with ${CMAKE_C_COMPILER} (build type=${CMAKE_BUILD_TYPE})")
