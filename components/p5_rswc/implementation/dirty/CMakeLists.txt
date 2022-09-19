cmake_minimum_required(VERSION 3.3)

include("p5.rwsc.romfs")
include("p5.rwsc.namespace")
include("p5.rwsc.recursive_sources")

unset("_base_name" CACHE)
mark_as_advanced("_base_name")
get_filename_component("_base_name" "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

unset("_namespace" CACHE)
mark_as_advanced("_namespace")
get_property("_namespace" DIRECTORY "." PROPERTY "p5.rwsc.namespace::name")

unset("_parent_namespace" CACHE)
mark_as_advanced("_parent_namespace")
get_property("_parent_namespace" DIRECTORY ".." PROPERTY "p5.rwsc.namespace::name")

add_library("${_namespace}" STATIC "src/dirty.cxx")

cmake_language(CALL "p5.rwsc.recursive_sources::add" "${_namespace}" "include")

add_library("${_parent_namespace}::${_base_name}" ALIAS "${_namespace}")

target_compile_features("${_namespace}" PUBLIC "cxx_std_20")

set_target_properties("${_namespace}" PROPERTIES
    "C_VISIBILITY_PRESET" "hidden"
    "CXX_VISIBILITY_PRESET" "hidden"
    "VISIBILITY_INLINES_HIDDEN" TRUE
    "POSITION_INDEPENDENT_CODE" TRUE
)

target_link_options("${_namespace}" PRIVATE "-Wl,-no-undefined")

target_link_libraries("${_namespace}" PRIVATE "fmt::fmt")
target_link_libraries("${_namespace}" PRIVATE
    "idf::nvs_flash"
    "idf::esp_http_server" "idf::esp_event"
    "idf::esp_netif" "idf::esp_hw_support" "idf::esp_system" "idf::esp_common"
)
target_link_libraries("${_namespace}" PRIVATE
    "${_parent_namespace}::romfs" "${_parent_namespace}::log" "${_parent_namespace}::common"
)

target_include_directories("${_namespace}" PUBLIC "include")
