cmake_minimum_required(VERSION 3.3)

include("p5.esp32.rwsc.romfs")
include("p5.esp32.rwsc.namespace")

cmake_language(CALL "p5.esp32.rwsc.romfs::add" "ssl/public.pem" "romfs/ssl/public.pem")
cmake_language(CALL "p5.esp32.rwsc.romfs::add" "ssl/private.pem" "romfs/ssl/private.pem")

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
    "src/implementation.cxx" "include/p5/rswc/implementation.hpp"
    "src/main_task.cxx" "src/main_task.hpp"
    "src/event_loop.cxx" "src/event_loop.hpp"
    "src/logged_action.cxx" "src/logged_action.hpp"
)

add_library("${_parent_namespace}::${_name}" ALIAS "${_namespace}")

target_compile_features("${_namespace}" PRIVATE "cxx_std_17")

set_target_properties("${_namespace}" PROPERTIES
    "C_VISIBILITY_PRESET" "hidden"
    "CXX_VISIBILITY_PRESET" "hidden"
    "VISIBILITY_INLINES_HIDDEN" TRUE
    "POSITION_INDEPENDENT_CODE" TRUE
)

target_link_options("${_namespace}" PRIVATE "-Wl,-no-undefined")

target_link_libraries("${_namespace}" PRIVATE "fmt::fmt")
target_link_libraries("${_namespace}" PRIVATE "idf::esp_common" "idf::esp_system" "idf::esp_event")
target_link_libraries("${_namespace}" PRIVATE
    "${_parent_namespace}::dirty"
    "${_parent_namespace}::romfs" "${_parent_namespace}::log" "${_parent_namespace}::common"
)

target_include_directories("${_namespace}" PUBLIC "include")
