add_library(program SHARED "${CMAKE_SOURCE_DIR}/include/model/entity/program.hpp" program.cpp)
target_include_directories(program PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(program PRIVATE unit errors table)
