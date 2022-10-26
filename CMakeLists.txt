# Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.20)
project(wait_all)

set(CMAKE_CXX_STANDARD 20)



add_library(wait_all INTERFACE)
add_library(Wait_all::wait_all ALIAS wait_all)


target_include_directories(wait_all INTERFACE include)

target_compile_features(wait_all INTERFACE cxx_std_20)

export(PACKAGE Wait_all)

enable_testing()
add_subdirectory(test)
