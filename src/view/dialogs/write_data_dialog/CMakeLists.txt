add_library(write_data_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/write_data_dialog.hpp" write_data_dialog.cpp)
qt_add_ui(write_data_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/write_data_dialog.ui")
target_include_directories(write_data_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(write_data_dialog PRIVATE controller repository program unit Qt6::Widgets)