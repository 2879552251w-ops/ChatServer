
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/client/*.cpp)

add_executable(ChatClient ${SRC})

target_link_libraries(ChatClient muduo_net pthread)