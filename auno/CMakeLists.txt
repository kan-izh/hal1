cmake_minimum_required(VERSION 2.6)

Project(ArduinoCode)

enable_testing()

add_subdirectory(libarduinocore)
add_subdirectory(lib)
add_subdirectory(firmware)
add_subdirectory(../3rd/gmock gmock)
add_subdirectory(test)
