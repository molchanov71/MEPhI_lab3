add_library(unit SHARED unit.cpp "${CMAKE_SOURCE_DIR}/include/model/entity/unit.hpp")
target_include_directories(unit PRIVATE "${CMAKE_SOURCE_DIR}/include")