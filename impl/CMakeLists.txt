cmake_minimum_required(VERSION 3.20)

project(binsalac LANGUAGES C CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BINSALAC_INSTALL "Install the binsalac tools" ON)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()


find_package(Boost REQUIRED COMPONENTS json)
find_package(LLVM REQUIRED CONFIG)

set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH Boost::json)
if(TARGET LLVM)
    set(LLVM_LIST_OF_LIBRARIES_TO_LINK_WITH LLVM)
else()
    llvm_map_components_to_libnames(LLVM_LIST_OF_LIBRARIES_TO_LINK_WITH all)
endif()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/libutility/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libsala/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libllvmutl/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/binsalac"
    ${LLVM_INCLUDE_DIRS}
)
add_definitions(${LLVM_DEFINITIONS})

if(BINSALAC_INSTALL)
    set(BINSALAC_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
endif()

add_subdirectory(libutility)
add_subdirectory(libsala)
add_subdirectory(libllvmutl)
add_subdirectory(binsalac)