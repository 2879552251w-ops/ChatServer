
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/server/*.cpp)

add_executable(muduoserver ${SRC})

find_library(MYSQL_LIB mysqlclient
    PATHS /usr/lib64/mysql
)
target_link_libraries(muduoserver muduo_net muduo_base pthread hiredis ${MYSQL_LIB})