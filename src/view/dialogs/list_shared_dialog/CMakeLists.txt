add_library(list_shared_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/list_shared_dialog.hpp" list_shared_dialog.cpp)
qt_add_ui(list_shared_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/list_shared_dialog.ui")
target_include_directories(list_shared_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(list_shared_dialog PRIVATE create_shared_dialog controller repository table Qt6::Widgets)