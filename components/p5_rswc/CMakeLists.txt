cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/p5_rswc.cxx" "include/p5_rswc.h" INCLUDE_DIRS "include")

list(APPEND "CMAKE_MODULE_PATH" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include("p5.rwsc.romfs")
include("p5.rwsc.namespace")
include("p5.rwsc.sdkconfig_test")

unset("_namespace" CACHE)
mark_as_advanced("_namespace")
get_property("_namespace" DIRECTORY "." PROPERTY "p5.rwsc.namespace::name")

unset("_sdkconfig_test_state" CACHE)
mark_as_advanced("_sdkconfig_test_state")
get_property("_sdkconfig_test_state" DIRECTORY "." PROPERTY "p5.rwsc.sdkconfig_test::state")

if("${_sdkconfig_test_state}")
    target_compile_definitions("${COMPONENT_LIB}" PRIVATE "SDKCONFIG_TEST_STATE=true")
    target_link_libraries("${COMPONENT_LIB}" PRIVATE "${_namespace}::implementation")
    add_subdirectory("implementation" EXCLUDE_FROM_ALL)
else()
    target_compile_definitions("${COMPONENT_LIB}" PRIVATE "SDKCONFIG_TEST_STATE=false")
    message(WARNING "unsupported project configuration (sdkconfig)")
endif()
