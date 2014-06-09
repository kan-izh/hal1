SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -pedantic")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${ArduinoCode_SOURCE_DIR}/libarduinocore)
include_directories(${ArduinoCode_SOURCE_DIR}/lib)
include_directories(${ArduinoCode_SOURCE_DIR}/auno)
include_directories(${NANOPB_DIR} ${NANOPB_GEN_DIR})

set(ScobsStreamTest_sources
	${ArduinoCode_SOURCE_DIR}/libarduinocore/Print.cpp
	${ArduinoCode_SOURCE_DIR}/lib/ScobsStream.cpp
	MockStream.h
	ScobsStreamTest.cpp
)
add_executable(ScobsStreamTest ${ScobsStreamTest_sources})
target_link_libraries(ScobsStreamTest gtest_main)
add_test(NAME ScobsStreamTest
	COMMAND ScobsStreamTest)

set(SignalPublisherTest_sources
	${ArduinoCode_SOURCE_DIR}/libarduinocore/Print.cpp
	${ArduinoCode_SOURCE_DIR}/lib/ScobsStream.cpp
	${NANOPB_DIR}/pb_decode.h
	${NANOPB_DIR}/pb_decode.c
	${NANOPB_DIR}/pb_encode.h
	${NANOPB_DIR}/pb_encode.c
	${NANOPB_GEN_DIR}/arduino.pb.c
	${NANOPB_GEN_DIR}/arduino.pb.h
	${ArduinoCode_SOURCE_DIR}/auno/SignalPublisher.cpp
	MockStream.h
	SignalPublisherTest.cpp
)
add_executable(SignalPublisherTest ${SignalPublisherTest_sources})
target_link_libraries(SignalPublisherTest gtest_main)
add_test(NAME SignalPublisherTest
	COMMAND SignalPublisherTest)
