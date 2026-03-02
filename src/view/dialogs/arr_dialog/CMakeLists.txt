add_library(arr_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/arr_dialog.hpp" arr_dialog.cpp)
qt_add_ui(arr_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/arr_dialog.ui")
target_include_directories(arr_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(arr_dialog PRIVATE write_data_dialog input_index_dialog read_index_dialog controller repository Qt6::Widgets)