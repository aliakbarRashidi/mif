cmake_minimum_required(VERSION 2.8)

project(Components)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

include (../common/cmake/mif.cmake)

#include_directories(${CMAKE_SOURCE_DIR}/common/include)
#include_directories(${CMAKE_SOURCE_DIR}/../common/include)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
#    ${CMAKE_SOURCE_DIR}/../common/include
#    ${CMAKE_SOURCE_DIR}/common/include
)

set(HEADERS
    ${HEADERS}
    ${COMMON_HEADERS}
)

set(SSOURCES
    ${SOURCES}
)

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_LC} ${LIBRARIES})
