include_directories(${NANOPB_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(proto_SOURCE_DIR ${ArduinoCode_SOURCE_DIR}/../interface/protobuf)
set(proto_FILE arduino.proto)

set(pb_generated_SOURCES
	arduino.pb.c
	arduino.pb.h
)

set(pb_sources
    ${pb_generated_SOURCES}
	${NANOPB_DIR}/pb_decode.h
	${NANOPB_DIR}/pb_decode.c
	${NANOPB_DIR}/pb_encode.h
	${NANOPB_DIR}/pb_encode.c
)

add_custom_command(
	OUTPUT
        ${pb_generated_SOURCES}
	DEPENDS ${proto_SOURCE_DIR}/${proto_FILE}
	WORKING_DIRECTORY ${proto_SOURCE_DIR}
	COMMAND protoc
		--nanopb_out=${CMAKE_CURRENT_BINARY_DIR}
		--plugin=protoc-gen-nanopb=${NANOPB_DIR}/generator/protoc-gen-nanopb
		${proto_FILE}
)
