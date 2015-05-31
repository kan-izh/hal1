Project(rf24-comm-lib)

include_directories(include)
include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${gmock_SOURCE_DIR}/include)

set(RingBufferPublisher_sources
	include/ByteBuffer.h
	include/RingBuffer.h
	include/RingBufferPublisher.h
	include/RingBufferSubscriber.h
)

set(RingBufferPublisherTest_sources
	${RingBufferPublisher_sources}
	RingBufferPublisherTest.cpp
	RingBufferSenderTest.cpp
)

add_executable(RingBufferPublisherTest ${RingBufferPublisherTest_sources})
target_link_libraries(RingBufferPublisherTest
	gtest_main
	gmock_main)
add_test(NAME RingBufferPublisherTest
	COMMAND RingBufferPublisherTest)

