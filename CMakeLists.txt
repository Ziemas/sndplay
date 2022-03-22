cmake_minimum_required(VERSION 3.1...3.22)

project(sndplay
  LANGUAGES CXX)

find_package(fmt REQUIRED)

add_subdirectory(third-party/cubeb)
add_subdirectory(src)
