cmake_minimum_required(VERSION 3.0)

include("FetchContent")

unset("_base_name" CACHE)
mark_as_advanced("_base_name")
get_filename_component("_base_name" "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

FetchContent_Declare(
  "${_base_name}"
  GIT_TAG "9.0.0"
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  OVERRIDE_FIND_PACKAGE
)

find_package("${_base_name}" REQUIRED)
