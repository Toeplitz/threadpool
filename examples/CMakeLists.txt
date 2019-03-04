project(threadpool)
include_directories(../src)

add_executable(example1 example1.cpp)
target_link_libraries(example1 threadpool ${PROJECT_LINK_LIBS})

add_executable(example2 example2.cpp)
target_link_libraries(example2 threadpool ${PROJECT_LINK_LIBS})