cmake_minimum_required(VERSION 3.0)

unset("_cmake_module_namespace" CACHE)
mark_as_advanced("_cmake_module_namespace")
get_filename_component("_cmake_module_namespace" "${CMAKE_CURRENT_LIST_FILE}" NAME_WLE)

function("${_cmake_module_namespace}::_initialize")
endfunction()

cmake_language(CALL "${_cmake_module_namespace}::_initialize")

cmake_language(EVAL CODE "function(\"${_cmake_module_namespace}::_initialize\")
    message(FATAL_ERROR \"unexpected call of ${_cmake_module_namespace}::_initialize\")
endfunction()")


function("${_cmake_module_namespace}::add" _target _path)
    unset("_paths" CACHE)
    mark_as_advanced("_paths")
    get_filename_component("_path" "${_path}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    file(GLOB_RECURSE "_paths" LIST_DIRECTORIES FALSE "${_path}/*")
    target_sources("${_target}" PRIVATE ${_paths})
endfunction()
