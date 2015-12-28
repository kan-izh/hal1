Project(RaspberryPi)
include(rpi.cmake)

include_directories(${RPI_RF24_DIR})
include_directories(${rf24-comm-lib_SOURCE_DIR}/include)

add_executable (auno-gw
    auno-gw.cpp

	${RPI_RF24_DIR}/arch/RPi/bcm2835.c
	${RPI_RF24_DIR}/arch/RPi/bcm2835.h
	${RPI_RF24_DIR}/arch/RPi/spi.cpp
	${RPI_RF24_DIR}/arch/RPi/spi.h
	${RPI_RF24_DIR}/RF24.cpp
	${RPI_RF24_DIR}/RF24.h
)
target_link_libraries(auno-gw rt)
