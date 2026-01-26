cmake_minimum_required(VERSION 3.11)

set(CPR_CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CPR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CPR_ENABLE_SSL ON CACHE BOOL "" FORCE)
set(CPR_FORCE_WINSSL_BACKEND ON CACHE BOOL "" FORCE)

project(CloudMusicLyrics)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cpr/CMakeLists.txt")
    add_subdirectory(cpr)
else()
    message(FATAL_ERROR "cpr dependency not found.")
endif()

file(GLOB_RECURSE VERSION_SOURCES
    "version/*.cpp"
    "version/*.h"
)

add_library(version SHARED ${VERSION_SOURCES})

target_include_directories(version PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/version"
    "${CMAKE_CURRENT_SOURCE_DIR}/version/3rd"
    "${CMAKE_CURRENT_SOURCE_DIR}/version/Hijack"
    "${CMAKE_CURRENT_SOURCE_DIR}/version/MinHook"
)

target_link_libraries(version PRIVATE
    cpr::cpr
    ws2_32
    crypt32
    wldap32
    advapi32
    d2d1
    dwrite
    psapi
)

target_compile_definitions(version PRIVATE UNICODE _UNICODE)

set_target_properties(version PROPERTIES 
    OUTPUT_NAME "version"
    PREFIX ""
)

if(MINGW)
    target_link_options(version PRIVATE "-static-libgcc" "-static-libstdc++")
    target_sources(version PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/version/version.def")
endif()
