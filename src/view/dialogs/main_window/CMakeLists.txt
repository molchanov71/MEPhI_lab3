add_library(main_window SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/main_window.hpp" main_window.cpp)
qt_add_ui(main_window SOURCES "${CMAKE_SOURCE_DIR}/include/ui/main_window.ui")
target_include_directories(main_window PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(main_window PRIVATE program_dialog create_program_dialog list_shared_dialog controller repository Qt6::Widgets)