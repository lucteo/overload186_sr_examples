cmake_minimum_required(VERSION 3.26)
project(Overload_sr_examples
  VERSION 0.1.0
  DESCRIPTION "Code examples for senders/receivers for Overload 186."
  LANGUAGES CXX
)

find_package(Threads REQUIRED)

set(EXECUTION_LIBRARY_PATH CACHE PATH "Path to Beman's execution library")
if (NOT EXECUTION_LIBRARY_PATH)
  message(FATAL_ERROR "Please set EXECUTION_LIBRARY_PATH to the path of stdexec")
endif()
if (NOT EXISTS "${EXECUTION_LIBRARY_PATH}/include/beman/execution/execution.hpp")
  message(FATAL_ERROR "${EXECUTION_LIBRARY_PATH}/include/beman/execution/execution.hpp does not exist")
endif()
message(STATUS "Using execution library from ${EXECUTION_LIBRARY_PATH}")

function(add_example name)
  add_executable(${name} "src/${name}.cpp")
  target_include_directories(${name} PUBLIC "${EXECUTION_LIBRARY_PATH}/include")
  target_compile_features(${name} PUBLIC cxx_std_20)
  target_link_libraries(${name} Threads::Threads)
endfunction()

add_example(just_int_test)
add_example(then_test)
add_example(serializer_test)
