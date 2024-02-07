cmake_minimum_required(VERSION 3.10)
project(Glad)

add_library(glad glad/glad.h "../src/glad.c")
target_include_directories(glad PUBLIC ./)