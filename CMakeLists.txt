cmake_minimum_required(VERSION 3.16)
project(instrfreq LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_WITH_LLVM "Enable building with LLVM" ON)

# ==========================
# Optional: LLVM libObject
# ==========================
find_package(LLVM REQUIRED CONFIG)

if (LLVM_FOUND AND BUILD_WITH_LLVM)
    message(STATUS "LLVM found: enabling universal binary loader")
    add_compile_definitions(USE_LLVM_LIBOBJECT=1)
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    llvm_map_components_to_libnames(LLVM_LIBS
        core
        support
        mc
        mcparser
        binaryformat
        object
    )
    link_directories(${LLVM_LIBARY_DIRS})
    link_libraries(${LLVM_LIBS})
endif(LLVM_FOUND AND BUILD_WITH_LLVM)


# ==========================
# Platform-native loaders
# ==========================
if (UNIX AND NOT APPLE)
    # Find libelf
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBELF REQUIRED libelf)
endif(UNIX AND NOT APPLE)

# Find Zydis (if installed with pkg-config)
#pkg_check_modules(ZYDIS REQUIRED Zydis)
add_subdirectory(external/zydis)

add_executable(instrfreq
    src/main.cpp
)

target_include_directories(instrfreq PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(instrfreq PRIVATE 
   Zydis
)

if (UNIX AND NOT APPLE)
    target_include_directories(instrfreq PRIVATE
        ${LIBELF_INCLUDE_DIRS}
    )

    target_link_directories(instrfreq PRIVATE
        ${LIBELF_LIBRARY_DIRS}
    )

    target_link_libraries(instrfreq PRIVATE
        ${LIBELF_LIBRARIES}
    )
endif(UNIX AND NOT APPLE)