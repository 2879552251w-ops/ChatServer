cmake_minimum_required(VERSION 3.10)
project(ChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall  -pthread -g)#-Wextra

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/server)
include_directories(${CMAKE_SOURCE_DIR}/include/server/db)
include_directories(${CMAKE_SOURCE_DIR}/include/server/model)
include_directories(${CMAKE_SOURCE_DIR}/include/server/redis)
link_directories(/usr/lib64/mysql)


add_subdirectory(src)

#file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR}/src/*cpp)

#find_library(MB NAMES muduo_base)
#find_library(muduo_net)

#add_executable(muduoserver ${SRC})
#target_link_libraries(muduoserver muduo_net muduo_base pthread)
#set_target_properties(muduoserver PROPERTIES
#RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")