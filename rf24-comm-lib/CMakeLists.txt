Project(rf24-comm-lib)

include_directories(include)
include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${gmock_SOURCE_DIR}/include)

set(RingBufferPublisher_sources
	include/ByteBuffer.h
	include/CommChannel.h
	include/RingBuffer.h
	include/RingBufferPublisher.h
	include/RingBufferSubscriber.h
)

set(CommLibTest_sources
	${RingBufferPublisher_sources}
	CommChannelTest.cpp
	RingBufferPublisherTest.cpp
	RingBufferSenderTest.cpp
)

add_executable(CommLibTest ${CommLibTest_sources})
target_link_libraries(CommLibTest
	gtest_main
	gmock_main)
add_test(NAME CommLibTest
	COMMAND CommLibTest)

