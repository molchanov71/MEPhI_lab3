add_library(create_program_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/create_program_dialog.hpp" create_program_dialog.cpp)
qt_add_ui(create_program_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/create_program_dialog.ui")
target_include_directories(create_program_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(create_program_dialog PRIVATE controller Qt6::Widgets)