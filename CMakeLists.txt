cmake_minimum_required (VERSION 3.9)
project (nix-command-store-plugin)
set (CMAKE_CXX_STANDARD 14)
set (nix-command-store-plugin_VERSION_MAJOR 3)
set (nix-command-store-plugin_VERSION_MINOR 0)

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,dynamic_lookup")

find_package(PkgConfig)

pkg_check_modules(NIX REQUIRED nix-store>=2.0)

add_library(nix-command-store SHARED command.cc command-store.cc)
add_definitions(-include config.h)
target_include_directories(nix-command-store PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(nix-command-store PUBLIC ${NIX_INCLUDE_DIRS})
target_compile_options(nix-command-store PUBLIC ${NIX_CFLAGS_OTHER})

install(TARGETS nix-command-store DESTINATION lib/nix/plugins)
