cmake_minimum_required(VERSION 2.6)

Project(ArduinoCode)

enable_testing()

SET(NANOPB_DIR ${ArduinoCode_SOURCE_DIR}/../3rd/nanopb)
SET(RF24_DIR ${ArduinoCode_SOURCE_DIR}/../3rd/RF24)

add_subdirectory(libarduinocore)
add_subdirectory(lib)
add_subdirectory(firmware)
add_subdirectory(../3rd/gtest-1.7.0 gtest)
add_subdirectory(test)
