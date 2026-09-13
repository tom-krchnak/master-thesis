add_library(salasym-lib STATIC)

set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/include/salasym")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/src")

target_sources(salasym-lib
    PUBLIC
        ${INC_DIR}/config.hpp
        ${INC_DIR}/expr.hpp
        ${INC_DIR}/types.hpp
    PRIVATE
        ${SRC_DIR}/expr.cpp

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
