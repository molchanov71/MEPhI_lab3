add_library(read_index_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/read_index_dialog.hpp" read_index_dialog.cpp)
qt_add_ui(read_index_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/read_index_dialog.ui")
target_include_directories(read_index_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(read_index_dialog PRIVATE Qt6::Widgets)