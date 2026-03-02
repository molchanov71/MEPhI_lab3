add_library(program_dialog SHARED "${CMAKE_SOURCE_DIR}/include/dialogs/program_dialog.hpp" program_dialog.cpp)
qt_add_ui(program_dialog SOURCES "${CMAKE_SOURCE_DIR}/include/ui/program_dialog.ui")
target_include_directories(program_dialog PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(program_dialog PRIVATE
        create_own_unit_dialog
        create_ref_dialog
        grant_shared_dialog
        var_dialog
        arr_dialog
        program_errors_dialog
        controller
        repository
        Qt6::Widgets
)