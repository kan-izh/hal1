include(${ArduinoCode_SOURCE_DIR}/arduino_diecimila_crosscompile.cmake)


add_library (arduinocore
	HardwareSerial.cpp
	cpp.c
	Print.cpp
	wiring.c
	wiring_shift.c
	WInterrupts.c
	wiring_digital.c
	WMath.cpp
	wiring_analog.c
	wiring_pulse.c
)
