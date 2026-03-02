add_library(repository SHARED "${CMAKE_SOURCE_DIR}/include/model/repository.hpp" repository.cpp)
target_include_directories(repository PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_directories(repository PRIVATE program unit)