include(${ArduinoCode_SOURCE_DIR}/arduino_diecimila_crosscompile.cmake)

SET(NANOPB_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated-sources/nanopb")
file(MAKE_DIRECTORY ${NANOPB_GEN_DIR})
include_directories(${ArduinoCode_SOURCE_DIR}/libarduinocore)
include_directories(${NANOPB_DIR} ${NANOPB_GEN_DIR})
LINK_DIRECTORIES(${ArduinoCode_SOURCE_DIR}/libarduinocore)

link_libraries(arduinocore)

add_custom_command(
	OUTPUT
		${NANOPB_GEN_DIR}/arduino.pb.c
		${NANOPB_GEN_DIR}/arduino.pb.h
	DEPENDS ${ArduinoCode_SOURCE_DIR}/../interface/protobuf/arduino.proto
	WORKING_DIRECTORY ${ArduinoCode_SOURCE_DIR}/../interface/protobuf
	COMMAND protoc
		--nanopb_out=${NANOPB_GEN_DIR}
		--plugin=protoc-gen-nanopb=${NANOPB_DIR}/generator/protoc-gen-nanopb
		arduino.proto
)

add_executable(firmware
	firmware.cpp
	${NANOPB_DIR}/pb_decode.h
	${NANOPB_DIR}/pb_decode.c
	${NANOPB_DIR}/pb_encode.h
	${NANOPB_DIR}/pb_encode.c
	${NANOPB_GEN_DIR}/arduino.pb.c
	${NANOPB_GEN_DIR}/arduino.pb.h
)

add_custom_command(
	OUTPUT firmware.hex
	COMMAND "${AVR_OBJCOPY}" -O ihex -R .eeprom firmware firmware.hex
	DEPENDS firmware
)
add_custom_target(hex
	ALL
	VERBATIM
	DEPENDS firmware.hex
)
