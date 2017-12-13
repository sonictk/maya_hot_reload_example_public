# CMake 2.8.11 and above required for linking to qtmain.lib library on Windows
# CMake 2.8.12 and above required for MACOSX_RPATH property
cmake_minimum_required(VERSION 2.8.12)
set(PROJECT_NAME "maya_hot_reload_example")
set(LOGIC_PLUGIN_NAME "logic")

project(${PROJECT_NAME})

# Attempt to find existing installation of Maya and define variables
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
find_package(Maya REQUIRED)

# Add include search paths
include_directories(
  ${MAYA_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/thirdparty
  ${PROJECT_SOURCE_DIR}/src
)
message(STATUS "Including Maya headers from: ${MAYA_INCLUDE_DIR}")

# Set compiler flags for each platform
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Setting compiler options for Clang...")
    # Using Clang
    set(CMAKE_CXX_FLAGS "-stdlib=libstdc++ \
                        -std=c++11 \
                        -fno-gnu-keywords \
                        -fpascal-strings \
                        -pthread \
                        -dynamic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O0 -gdwarf-4 -gmodules")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_SHARED_LINKER_FLAGS "-stdlib=libstdc++ \
                                -std=c++0x \
                                -fno-gnu-keywords \
                                -fpascal-strings \
                                -Wl, \
                                -headerpad_max_install_names \
                                -dynamic")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Setting compiler options for GCC...")
    # using GCC on Linux
    set(CMAKE_C_FLAGS "-DBits64_ \
                      -m64 \
                      -DUNIX \
                      -D_BOOL \
                      -DLINUX \
                      -DFUNCPROTO \
                      -D_GNU_SOURCE \
                      -DLINUX_64 \
                      -fPIC \
                      -fno-strict-aliasing \
                      -DREQUIRE_IOSTREAM \
                      -Wall \
                      -std=c++11 \
                      -Wno-multichar \
                      -Wno-comment \
                      -Wno-sign-compare \
                      -funsigned-char \
                      -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} \
                        -Wno-deprecated \
                        -Wno-reorder \
                        -ftemplate-depth-25 \
                        -fno-gnu-keywords")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -ggdb -O0")
    set(CMAKE_CXX_FLAGS_RELEASE"${CMAKE_CXX_FLAGS} -O3")

elseif(MSVC)
    message(STATUS "Setting compiler options for MSVC...")
    # using Visual Studio C++
    set(CMAKE_CXX_FLAGS "/MP /W3 /Gy /Zc:wchar_t /Zc:forScope /fp:precise /nologo /openmp /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Zi /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2")

    # NOTE: (sonictk) Disable MSVC warnings about ``strncpy``
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message(STATUS "Setting debug preprocessor definitions...")
    add_definitions("-D_DEBUG_MODE")
endif()

# Don't skip the full RPATH for the build/install tree
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# Disable RPATH stripping during installation so that the binary will link correctly
# to the Maya libs
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Get the parent installation directory if it exists; this is used to format
# the installation path for the plugins
get_filename_component(PLUGIN_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
if(IS_DIRECTORY ${PLUGIN_ROOT_DIR}/bin)
    set(CMAKE_INSTALL_PREFIX ${PLUGIN_ROOT_DIR}/bin)
else()
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    message(WARNING "Could not find binaries install directory, installing to default one!")
endif()

# NOTE: (sonictk) This makes the sources available in IDEs like Visual Studio
set(PLUGIN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/deformer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/deformer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/deformer_platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/deformer_platform.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plugin_main.h")
set(PLUGIN_ENTRY_POINT "${CMAKE_CURRENT_SOURCE_DIR}/src/plugin_main.cpp")

set(LOGIC_PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/logic.h")
set(LOGIC_PLUGIN_ENTRY_POINT "${CMAKE_CURRENT_SOURCE_DIR}/src/logic.cpp")

set_source_files_properties(${PLUGIN_SOURCES} ${LOGIC_PLUGIN_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

# Add targets to build
add_library(${LOGIC_PLUGIN_NAME} SHARED ${LOGIC_PLUGIN_ENTRY_POINT})
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_ENTRY_POINT})

# Link targets to libraries
message(STATUS "Linking to Maya libraries at: ${MAYA_LIBRARY_DIR}")
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES})
target_link_libraries(${LOGIC_PLUGIN_NAME} ${MAYA_LIBRARIES})

# Any OS-specific library linking goes here
if(WIN32)
    target_link_libraries(${PROJECT_NAME} "Shlwapi.dll")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
endif()

# Set preprocessor definitions
MAYA_PLUGIN(${PROJECT_NAME})

# NOTE: (yliangsiew) Make sure that the shared library is named without any annoying prefixes
if(WIN32)
    set_target_properties(${LOGIC_PLUGIN_NAME} PROPERTIES PREFIX "" SUFFIX ".dll")
elseif(APPLE OR UNIX)
    set_target_properties(${LOGIC_PLUGIN_NAME} PROPERTIES PREFIX "" SUFFIX ".so")
endif()

# NOTE: (sonictk) Because of Windows locking the DLL (Linux doesn't
# actaully remove the file itself, only the inode), we'll rename the
# DLL first if it already exists, then delete it after.
add_custom_command(TARGET "${LOGIC_PLUGIN_NAME}" PRE_BUILD COMMAND ${CMAKE_COMMAND}
    -DLOGIC_LIB_NAME=$<TARGET_FILE:${LOGIC_PLUGIN_NAME}>
    -P "${PROJECT_SOURCE_DIR}/scripts/renameLogicLib.cmake"
    COMMENT "Running rename library script..." VERBATIM)

add_custom_command(TARGET "${LOGIC_PLUGIN_NAME}" POST_BUILD COMMAND ${CMAKE_COMMAND}
    -DDELETE_LIB_NAME="$<TARGET_FILE:${LOGIC_PLUGIN_NAME}>.temp"
    -P "${PROJECT_SOURCE_DIR}/scripts/deleteTmpLogicLib.cmake"
    COMMENT "Running deletion script..." VERBATIM)

install(TARGETS ${PROJECT_NAME} ${LOGIC_PLUGIN_NAME} ${MAYA_TARGET_TYPE} DESTINATION ${CMAKE_INSTALL_PREFIX})
