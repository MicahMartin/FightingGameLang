project(virtual_machine)

cmake_minimum_required(VERSION 3.9)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCE src/*.cpp)
file(GLOB_RECURSE HEADER include/*.h)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(virtual_machine ${SOURCE} ${HEADER})

target_link_libraries(
   virtual_machine 
)

