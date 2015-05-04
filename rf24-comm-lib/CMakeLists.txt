Project(rf24-comm-lib)

include_directories(include)
include_directories(${gtest_SOURCE_DIR}/include)

set(RingBufferPublisher_sources
	include/ByteBuffer.h
	include/RingBufferPublisher.h
)

set(RingBufferPublisherTest_sources
	${RingBufferPublisher_sources}
	RingBufferPublisherTest.cpp
)

add_executable(RingBufferPublisherTest ${RingBufferPublisherTest_sources})
target_link_libraries(RingBufferPublisherTest gtest_main)
add_test(NAME RingBufferPublisherTest
	COMMAND RingBufferPublisherTest)

