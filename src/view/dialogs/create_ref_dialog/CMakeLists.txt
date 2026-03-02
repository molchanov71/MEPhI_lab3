add_library(create_ref_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/create_ref_dialog.hpp" create_ref_dialog.cpp)
qt_add_ui(create_ref_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/create_ref_dialog.ui")
target_include_directories(create_ref_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(create_ref_dialog PRIVATE controller Qt6::Widgets)