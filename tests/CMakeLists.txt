find_package(Catch2 3 CONFIG REQUIRED)

add_executable(tests
        tests.cpp
)
include_directories(tests PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(tests
        PRIVATE
        controller
        service
        repository
        program
        unit
        Catch2::Catch2WithMain
)