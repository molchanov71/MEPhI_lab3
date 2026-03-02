add_library(create_shared_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/create_shared_dialog.hpp" create_shared_dialog.cpp)
qt_add_ui(create_shared_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/create_shared_dialog.ui")
target_include_directories(create_shared_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(create_shared_dialog PRIVATE controller Qt6::Widgets)