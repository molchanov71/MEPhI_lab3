add_library(input_index_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/input_index_dialog.hpp" input_index_dialog.cpp)
qt_add_ui(input_index_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/input_index_dialog.ui")
target_include_directories(input_index_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(input_index_dialog PRIVATE Qt6::Widgets)