cmake_minimum_required(VERSION 2.6)

Project(HAL1)

SET(NANOPB_DIR ${HAL1_SOURCE_DIR}/3rd/nanopb)
SET(RF24_DIR ${HAL1_SOURCE_DIR}/3rd/RF24)
SET(RPI_RF24_DIR ${RF24_DIR})

enable_testing()

add_subdirectory(auno)
add_subdirectory(rf24-comm-lib)
add_subdirectory(rpi)
