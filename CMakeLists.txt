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

# Find dependencies
find_package(CURL REQUIRED)

# Make sure the project uses our include directory and libgit2
target_include_directories(iris PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/vendor/libgit2-1.8.5/include")


# Build and link libgit2 statically from vendor
add_subdirectory(vendor/libgit2-1.8.5 vendor/libgit2-1.8.5/build EXCLUDE_FROM_ALL)
target_link_libraries(iris PRIVATE CURL::libcurl git2)

# Compiler flags (add -Wall by default)
target_compile_options(iris PRIVATE -Wall)


set_target_properties(iris PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# Write project root to iris.conf in $HOME/.config/iris-cli during build
add_custom_command(
  TARGET iris POST_BUILD
  COMMAND mkdir -p "$ENV{HOME}/.config/iris-cli" && echo "PATH=${PROJECT_SOURCE_DIR}" > "$ENV{HOME}/.config/iris-cli/iris.conf"
  COMMENT "Writing project root to $HOME/.config/iris-cli/iris.conf"
)

# Install target (optional)
install(TARGETS iris
  RUNTIME DESTINATION bin
)

# Download and extract libgit2 if not present
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/vendor/libgit2-v1.8.5.zip")
  file(DOWNLOAD "https://github.com/libgit2/libgit2/archive/refs/tags/v1.8.5.zip" "${PROJECT_SOURCE_DIR}/vendor/libgit2-v1.8.5.zip" SHOW_PROGRESS)
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/vendor/libgit2-1.8.5/CMakeLists.txt")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${PROJECT_SOURCE_DIR}/vendor/libgit2-v1.8.5.zip" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/vendor")
endif()

message(STATUS "Configured IrisCLI with ${CMAKE_C_COMPILER} (build type=${CMAKE_BUILD_TYPE})")
