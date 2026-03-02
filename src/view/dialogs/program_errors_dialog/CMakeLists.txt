add_library(program_errors_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/program_errors_dialog.hpp" program_errors_dialog.cpp)
qt_add_ui(program_errors_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/program_errors_dialog.ui")
target_include_directories(program_errors_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(program_errors_dialog PRIVATE repository Qt6::Widgets)