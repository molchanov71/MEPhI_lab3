add_library(create_own_unit_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/create_own_unit_dialog.hpp" create_own_unit_dialog.cpp)
qt_add_ui(create_own_unit_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/create_own_unit_dialog.ui")
target_include_directories(create_own_unit_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(create_own_unit_dialog PRIVATE controller Qt6::Widgets)