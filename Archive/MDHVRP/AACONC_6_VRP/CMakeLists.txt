cmake_minimum_required(VERSION 3.28.1)

project(AACONC)

find_package(OpenMP REQUIRED)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.c")

add_executable(${PROJECT_NAME} main.c ${SOURCES})

include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/header_files)

target_link_libraries(${PROJECT_NAME} m OpenMP::OpenMP_C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
