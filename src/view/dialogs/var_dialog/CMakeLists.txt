add_library(var_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/var_dialog.hpp" var_dialog.cpp)
qt_add_ui(var_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/var_dialog.ui")
target_include_directories(var_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(var_dialog PRIVATE write_data_dialog controller repository unit Qt6::Widgets)