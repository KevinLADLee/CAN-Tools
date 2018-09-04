cmake_minimum_required(VERSION 3.2.0)
project(linux_can_test)

set(CMAKE_CXX_STANDARD 11)

add_library(can_device_linux can_device_linux.cpp can_device_linux.h)

