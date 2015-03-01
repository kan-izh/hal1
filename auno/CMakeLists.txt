cmake_minimum_required(VERSION 2.6)

Project(ArduinoCode)

enable_testing()

add_subdirectory(libarduinocore)
add_subdirectory(lib)
add_subdirectory(firmware)
add_subdirectory(../3rd/gtest-1.7.0 gtest)
add_subdirectory(test)
