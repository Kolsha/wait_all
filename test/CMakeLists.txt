# Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
cmake_minimum_required(VERSION 3.20)

include(CTest)

add_executable(unittest)
target_sources(
        unittest
        PRIVATE
        common.h
        wait_all.cpp
        wait_all_stress.cpp
)

target_include_directories(
        unittest
        PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>/include
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

target_compile_features(unittest PRIVATE cxx_std_20)

set_target_properties(unittest PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")

find_package(GTest REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
# set(Boost_DEBUG 1)
find_package(Boost 1.76.0 REQUIRED COMPONENTS system context thread coroutine)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

target_link_libraries(
        unittest
        GTest::GTest
        GTest::Main
        GTest::gmock
        ${Boost_LIBRARIES}
)

add_test(NAME unittest COMMAND unittest)
