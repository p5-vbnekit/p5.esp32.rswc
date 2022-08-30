cmake_minimum_required(VERSION 3.3)

include("p5.esp32.rwsc.namespace")

add_subdirectory("main")
add_subdirectory("dirty")
add_subdirectory("romfs")
add_subdirectory("log")
add_subdirectory("common")
add_subdirectory("3rd-party")

unset("_base_name" CACHE)
mark_as_advanced("_base_name")
get_filename_component("_base_name" "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

unset("_namespace" CACHE)
mark_as_advanced("_namespace")
get_property("_namespace" DIRECTORY "." PROPERTY "p5.esp32.rwsc.namespace::name")

unset("_parent_namespace" CACHE)
mark_as_advanced("_parent_namespace")
get_property("_parent_namespace" DIRECTORY ".." PROPERTY "p5.esp32.rwsc.namespace::name")

add_library("${_namespace}" ALIAS "${_namespace}.main")
add_library("${_parent_namespace}::${_base_name}" ALIAS "${_namespace}.main")
