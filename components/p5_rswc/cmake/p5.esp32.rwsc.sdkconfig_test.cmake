cmake_minimum_required(VERSION 3.0)

unset("_cmake_module_namespace" CACHE)
mark_as_advanced("_cmake_module_namespace")
get_filename_component("_cmake_module_namespace" "${CMAKE_CURRENT_LIST_FILE}" NAME_WLE)

function("${_cmake_module_namespace}::_initialize")
    unset("_state" CACHE)
    unset("_script_base_name" CACHE)
    mark_as_advanced("_state")
    mark_as_advanced("_script_base_name")
    get_filename_component("_script_base_name" "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" NAME_WLE)
    set("_state" TRUE)
    if(NOT "${CONFIG_COMPILER_CXX_RTTI}" STREQUAL "y")
        set("_state" FALSE)
        message(WARNING "CONFIG_COMPILER_CXX_RTTI is required")
    endif()
    if(NOT "${CONFIG_COMPILER_CXX_EXCEPTIONS}" STREQUAL "y")
        set("_state" FALSE)
        message(WARNING "CONFIG_COMPILER_CXX_EXCEPTIONS is required")
    endif()
    set_property(DIRECTORY "." PROPERTY "${_script_base_name}::state" "${_state}")
endfunction()

cmake_language(CALL "${_cmake_module_namespace}::_initialize")

cmake_language(EVAL CODE "function(\"${_cmake_module_namespace}::_initialize\")
    message(FATAL_ERROR \"unexpected call of ${_cmake_module_namespace}::_initialize\")
endfunction()")
