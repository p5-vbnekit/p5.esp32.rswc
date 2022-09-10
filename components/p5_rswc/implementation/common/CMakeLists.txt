cmake_minimum_required(VERSION 3.3)

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
    "src/common.cxx" "include/p5/rswc/implementation_/common.hpp"
    "src/common.sdk.cxx" "include/p5/rswc/implementation_/common/sdk.hpp"
    "src/common.coro.cxx" "include/p5/rswc/implementation_/common/coro.hpp"
    "src/common.notifier.cxx" "include/p5/rswc/implementation_/common/notifier.hpp"
    "src/common.exception_handling.cxx" "include/p5/rswc/implementation_/common/exception_handling.hpp"
    "src/common.utils.cxx" "include/p5/rswc/implementation_/common/utils.hpp"
    "include/p5/rswc/implementation_/common/coro/future.hpp"
    "include/p5/rswc/implementation_/common/coro/task.hpp"
    "include/p5/rswc/implementation_/common/coro/private_.hpp"
    "include/p5/rswc/implementation_/common/notifier/fwd.hpp"
)

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

target_link_libraries("${_namespace}" PRIVATE "idf::esp_common")

target_include_directories("${_namespace}" PUBLIC "include")
