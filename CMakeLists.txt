# Set libgit2 version as a variable
set(LIBGIT2_VERSION 1.8.5)
cmake_minimum_required(VERSION 3.10)

project(ErisCLI VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default to Debug if not provided
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release)" FORCE)
endif()

# Define project source files
file(GLOB_RECURSE ERISCLI_SRC
  "${PROJECT_SOURCE_DIR}/src/*.c"
  "${PROJECT_SOURCE_DIR}/lib/**/*.c"
)

add_executable(eris ${ERISCLI_SRC})

# Find dependencies
find_package(CURL REQUIRED)

# SWI-Prolog (embedded) is required; no C fallback.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(SWIPL QUIET swipl)
endif()
if(NOT SWIPL_FOUND)
  find_library(SWIPL_LIBRARY NAMES swipl)
  find_path(SWIPL_INCLUDE_DIR NAMES SWI-Prolog.h)
  if(SWIPL_LIBRARY AND SWIPL_INCLUDE_DIR)
    set(SWIPL_LIBRARIES ${SWIPL_LIBRARY})
    set(SWIPL_INCLUDE_DIRS ${SWIPL_INCLUDE_DIR})
    set(SWIPL_FOUND TRUE)
  endif()
endif()
# Prefer full path for linking so the linker always finds libswipl
if(SWIPL_FOUND AND NOT SWIPL_LIBRARY)
  find_library(SWIPL_LIBRARY NAMES swipl)
endif()
if(SWIPL_FOUND)
  target_compile_definitions(eris PRIVATE ERIS_USE_PROLOG=1)
  target_include_directories(eris PRIVATE ${SWIPL_INCLUDE_DIRS})
  if(SWIPL_LIBRARY_DIRS)
    target_link_directories(eris PRIVATE ${SWIPL_LIBRARY_DIRS})
  endif()
  if(SWIPL_LIBRARY)
    target_link_libraries(eris PRIVATE ${SWIPL_LIBRARY})
  else()
    target_link_libraries(eris PRIVATE ${SWIPL_LIBRARIES})
  endif()
  message(STATUS "Eris: using Prolog command router (SWI-Prolog)")
else()
  message(FATAL_ERROR "Eris requires SWI-Prolog. Install it (e.g. libswipl-dev on Debian/Ubuntu, swi-prolog on Fedora/Arch) and re-run CMake.")
endif()

# Make sure the project uses our include directories
target_include_directories(eris PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/lib")

# libgit2: prefer the distro-packaged system library so package builds need no
# network (works in offline/clean-chroot builds — PPA, makechrootpkg). Only fall
# back to compiling the vendored source when no system libgit2 is available.
set(LIBGIT2_DIR vendor/libgit2-${LIBGIT2_VERSION})
set(_eris_sys_git2 OFF)
if(PkgConfig_FOUND)
  pkg_check_modules(LIBGIT2 IMPORTED_TARGET libgit2)
  if(LIBGIT2_FOUND)
    set(ERIS_GIT2_LIB PkgConfig::LIBGIT2)
    set(_eris_sys_git2 ON)
  endif()
endif()
if(NOT _eris_sys_git2)
  find_library(GIT2_LIBRARY git2)
  find_path(GIT2_INCLUDE_DIR git2.h)
  if(GIT2_LIBRARY AND GIT2_INCLUDE_DIR)
    target_include_directories(eris PRIVATE ${GIT2_INCLUDE_DIR})
    set(ERIS_GIT2_LIB ${GIT2_LIBRARY})
    set(_eris_sys_git2 ON)
  endif()
endif()
if(_eris_sys_git2)
  message(STATUS "Eris: using system libgit2")
else()
  message(STATUS "Eris: system libgit2 not found; building vendored ${LIBGIT2_VERSION}")
  set(LIBGIT2_ZIP vendor/libgit2-v${LIBGIT2_VERSION}.zip)
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${LIBGIT2_DIR}/CMakeLists.txt")
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${LIBGIT2_ZIP}")
      execute_process(COMMAND wget -O "${PROJECT_SOURCE_DIR}/${LIBGIT2_ZIP}" "https://github.com/libgit2/libgit2/archive/refs/tags/v${LIBGIT2_VERSION}.zip")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${PROJECT_SOURCE_DIR}/${LIBGIT2_ZIP}" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/vendor")
  endif()
  add_subdirectory(${LIBGIT2_DIR})
  target_include_directories(eris PRIVATE ${PROJECT_SOURCE_DIR}/${LIBGIT2_DIR}/include)
  set(ERIS_GIT2_LIB git2)
endif()

find_library(READLINE_LIBRARY NAMES readline REQUIRED)
target_link_libraries(eris PRIVATE ${ERIS_GIT2_LIB} CURL::libcurl ${READLINE_LIBRARY})

# Compiler flags (add -Wall by default)
target_compile_options(eris PRIVATE -Wall)


set_target_properties(eris PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# Write project root to eris.conf in $HOME/.config/eris-cli during build (optional; do not fail the build)
add_custom_command(
  TARGET eris POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E env HOME=$ENV{HOME} sh -c "mkdir -p \"$$HOME/.config/eris-cli\" && echo 'PATH=${PROJECT_SOURCE_DIR}' > \"$$HOME/.config/eris-cli/eris.conf\"" || true
  COMMENT "Writing project root to $HOME/.config/eris-cli/eris.conf (optional)"
)

# Install target (optional)
install(TARGETS eris
  RUNTIME DESTINATION bin
)

message(STATUS "Configured ErisCLI with ${CMAKE_C_COMPILER} (build type=${CMAKE_BUILD_TYPE})")
