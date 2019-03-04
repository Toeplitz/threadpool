project(threadpool)
include_directories(../src)

file(GLOB SOURCES "*.cpp")

add_executable(example ${SOURCES})
target_link_libraries(example threadpool ${PROJECT_LINK_LIBS})