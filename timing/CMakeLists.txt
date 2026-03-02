add_executable(timing timing.cpp)
include_directories(timing PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(timing PRIVATE service repository program)