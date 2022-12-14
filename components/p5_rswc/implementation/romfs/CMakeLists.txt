cmake_minimum_required(VERSION 3.3)

include("p5.esp32.rwsc.romfs")
include("p5.esp32.rwsc.namespace")

unset("_base_name" CACHE)
mark_as_advanced("_base_name")
get_filename_component("_base_name" "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

unset("_namespace" CACHE)
mark_as_advanced("_namespace")
get_property("_namespace" DIRECTORY "." PROPERTY "p5.esp32.rwsc.namespace::name")

unset("_parent_namespace" CACHE)
mark_as_advanced("_parent_namespace")
get_property("_parent_namespace" DIRECTORY ".." PROPERTY "p5.esp32.rwsc.namespace::name")

add_library("${_namespace}" STATIC
    "src/romfs.cxx" "include/p5/rswc/implementation_/romfs.hpp"
)

add_library("${_parent_namespace}::${_base_name}" ALIAS "${_namespace}")

target_compile_features("${_namespace}" PRIVATE "cxx_std_17")

set_target_properties("${_namespace}" PROPERTIES
    "C_VISIBILITY_PRESET" "hidden"
    "CXX_VISIBILITY_PRESET" "hidden"
    "VISIBILITY_INLINES_HIDDEN" TRUE
    "POSITION_INDEPENDENT_CODE" TRUE
)

target_link_options("${_namespace}" PRIVATE "-Wl,-no-undefined")

target_link_libraries("${_namespace}" PRIVATE "fmt::fmt")
target_link_libraries("${_namespace}" PRIVATE "idf::json")
target_link_libraries("${_namespace}" PRIVATE "p5_rswc.romfs_image_layout")

target_include_directories("${_namespace}" PUBLIC "include")
