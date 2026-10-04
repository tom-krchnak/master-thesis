cmake_minimum_required(VERSION 3.20)

project(salasym LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS json)
find_package(LLVM REQUIRED CONFIG)

set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH Boost::json)
if(TARGET LLVM)
    set(SALASYM_LLVM_LIBRARIES LLVM)
else()
    llvm_map_components_to_libnames(SALASYM_LLVM_LIBRARIES Support)
endif()

add_definitions(${LLVM_DEFINITIONS})
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/libutility/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libsala/include"
)
add_subdirectory(libutility)
add_subdirectory(libsala)
add_subdirectory(salasym)
