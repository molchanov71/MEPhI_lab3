add_library(controller SHARED "${CMAKE_SOURCE_DIR}/include/controller.hpp" controller.cpp)
target_include_directories(controller PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(controller PRIVATE service)
