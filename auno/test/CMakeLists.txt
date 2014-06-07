SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -pedantic")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${ArduinoCode_SOURCE_DIR}/libarduinocore)
include_directories(${ArduinoCode_SOURCE_DIR}/lib)

set(ScobsStream_sources
	${ArduinoCode_SOURCE_DIR}/libarduinocore/Print.cpp
	${ArduinoCode_SOURCE_DIR}/lib/ScobsStream.cpp
	ScobsStreamTest.cpp
)
add_executable(ScobsStream ${ScobsStream_sources})
target_link_libraries(ScobsStream gtest_main)
add_test(NAME ScobsStream
	COMMAND ScobsStream)
