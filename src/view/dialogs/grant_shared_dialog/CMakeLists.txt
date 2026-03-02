add_library(grant_shared_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/grant_shared_dialog.hpp" grant_shared_dialog.cpp)
qt_add_ui(grant_shared_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/grant_shared_dialog.ui")
target_include_directories(grant_shared_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(grant_shared_dialog PRIVATE controller repository Qt6::Widgets)