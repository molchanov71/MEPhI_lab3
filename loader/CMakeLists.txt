add_library(loader SHARED loader.hpp loader.cpp)
target_include_directories(loader PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(loader PRIVATE view controller service repository)