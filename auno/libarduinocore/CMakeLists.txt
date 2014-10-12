include(${ArduinoCode_SOURCE_DIR}/arduino_diecimila_crosscompile.cmake)
include_directories(${RF24_DIR})
ADD_DEFINITIONS(-DARDUINO=100)

add_library (arduinocore
	Arduino.h
	Arduino_private.h
	HardwareSerial.cpp
	new.cpp
	new.h
	Print.cpp
	wiring.c
	wiring_shift.c
	WInterrupts.c
	wiring_digital.c
	WMath.cpp
	wiring_analog.c
	wiring_pulse.c
	sos.h
	sos.c
	SPI.h
	SPI.cpp
	Rf24Stream.h
	Rf24Stream.cpp
)
