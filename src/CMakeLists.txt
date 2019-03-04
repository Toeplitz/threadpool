project(threadpool)

file(GLOB SOURCES "*.cpp")

set(PROJECT_LINK_LIBS "-pthread")

add_library(threadpool STATIC ${SOURCES})
target_link_libraries(threadpool ${PROJECT_LINK_LIBS})