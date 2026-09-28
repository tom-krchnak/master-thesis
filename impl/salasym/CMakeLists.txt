add_library(salasym-lib STATIC)

set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/include/salasym")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/src")

target_sources(salasym-lib
    PUBLIC
        ${INC_DIR}/config.hpp
        ${INC_DIR}/exec.hpp
        ${INC_DIR}/expr.hpp
        ${INC_DIR}/memory.hpp
        ${INC_DIR}/outcome.hpp
        ${INC_DIR}/path_cond.hpp
        ${INC_DIR}/search.hpp
        ${INC_DIR}/search/dfs.hpp
        ${INC_DIR}/state_store.hpp
        ${INC_DIR}/state.hpp
        ${INC_DIR}/stepper.hpp
        ${INC_DIR}/types.hpp
    PRIVATE
        ${SRC_DIR}/exec.cpp
        ${SRC_DIR}/expr.cpp
        ${SRC_DIR}/search.cpp
        ${SRC_DIR}/stepper.cpp

)

target_include_directories(salasym-lib
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/include"
)

target_link_libraries(salasym-lib
    PUBLIC
        sala
        utility
        ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
)

add_executable(salasym)

target_sources(salasym
    PRIVATE
        app/main.cpp
)

target_link_libraries(salasym
    PRIVATE
        salasym-lib
)
