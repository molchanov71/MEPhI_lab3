add_library(service SHARED "${CMAKE_SOURCE_DIR}/include/model/service.hpp" service.cpp)
target_include_directories(service PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(service PRIVATE repository program unit errors)