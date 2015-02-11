include(${ArduinoCode_SOURCE_DIR}/arduino_diecimila_crosscompile.cmake)
include_directories(${ArduinoCode_SOURCE_DIR}/libarduinocore)

add_library (lib
	ScobsStream.h
	ScobsStream.cpp
	RingBufferPublisher.h
)
