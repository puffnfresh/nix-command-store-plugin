cmake_minimum_required (VERSION 3.9)
project (nix-script-store-plugin)
set (CMAKE_CXX_STANDARD 14)
set (nix-script-store-plugin_VERSION_MAJOR 3)
set (nix-script-store-plugin_VERSION_MINOR 0)

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,dynamic_lookup")

find_package(PkgConfig)

pkg_check_modules(NIX REQUIRED nix-store>=2.0)

add_library(nix-script-store SHARED script.cc script-store.cc)
add_definitions(-include config.h)
target_include_directories(nix-script-store PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(nix-script-store PUBLIC ${NIX_INCLUDE_DIRS})
target_compile_options(nix-script-store PUBLIC ${NIX_CFLAGS_OTHER})

install(TARGETS nix-script-store DESTINATION lib/nix/plugins)
