cmake_minimum_required(VERSION 3.16)
project(ProtoTableTests)

enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Test REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(ProtoTableTests
    test_prototablemodel.cpp
)

target_link_libraries(ProtoTableTests
    PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Qml
    gtest_main
    prototablemodel
)

include(GoogleTest)
gtest_discover_tests(ProtoTableTests)
add_test(NAME ProtoTableTests COMMAND ProtoTableTests)