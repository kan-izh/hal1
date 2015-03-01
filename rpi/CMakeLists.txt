Project(RaspberryPi)
include(rpi.cmake)

include_directories(${RPI_RF24_DIR})

add_executable (auno-gw
    auno-gw.cpp

	${RPI_RF24_DIR}/bcm2835.c
	${RPI_RF24_DIR}/RF24.cpp
	${RPI_RF24_DIR}/RF24.h
)
