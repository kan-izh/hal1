include(${ArduinoCode_SOURCE_DIR}/arduino_diecimila_crosscompile.cmake)

include(${ArduinoCode_SOURCE_DIR}/pb.cmake)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

include_directories(${ArduinoCode_SOURCE_DIR}/libarduinocore)
include_directories(${ArduinoCode_SOURCE_DIR}/lib)
include_directories(${RF24_DIR})
include_directories(${rf24-comm-lib_SOURCE_DIR}/include)
LINK_DIRECTORIES(${ArduinoCode_SOURCE_DIR}/libarduinocore)
ADD_DEFINITIONS(-DARDUINO=100)

link_libraries(arduinocore)
link_libraries(lib)

add_executable(firmware.elf
	${pb_sources}
	firmware.cpp
	SignalPublisher.h
	SignalPublisher.cpp
	rf-settings.h

	${RF24_DIR}/RF24.cpp
	${RF24_DIR}/RF24.h
)

add_custom_command(
	OUTPUT firmware.hex
	COMMAND "${AVR_OBJCOPY}" -O ihex -R .eeprom firmware.elf -v firmware.hex
	DEPENDS firmware.elf
)
add_custom_target(firmware
	ALL
	VERBATIM
	DEPENDS firmware.hex
)
