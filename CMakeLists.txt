cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(dnsRelay LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

add_executable(dnsRelay "")

target_sources(dnsRelay
  PRIVATE
    dns.c
    init.c
    query.c
    request.c
  )

if(WIN32)
  target_link_libraries(dnsRelay wsock32 ws2_32)
endif()