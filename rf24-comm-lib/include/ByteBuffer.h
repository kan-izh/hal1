#ifndef HAL1_SERIALISER_H
#define HAL1_SERIALISER_H

#include <stdint.h>

template<uint8_t size>
class ByteBuffer
{
private:
	uint8_t buf[size];
public:
	class Accessor
	{
	private:
		typedef ByteBuffer<size> Buffer;
		Buffer *buffer;
		uint8_t offset;
	public:
		Accessor(Buffer *buffer)
		{
			reset(buffer);
		}

		void reset(Buffer *buffer)
		{
			this->buffer = buffer;
			offset = 0;
		}

	public:
		template<typename T>
		const T & get(uint8_t offset) const
		{
			const T &value = *reinterpret_cast<const T *>(getBuf() + offset);
			return value;
		}

		template<typename T>
		const T & take()
		{
			const T &value = *reinterpret_cast<const T *>(getBuf() + offset);
			offset += sizeof(T);
			return value;
		}

		template<typename T>
		Accessor &put(const T &value)
		{
			*reinterpret_cast<T *>(buffer->getBuf() + offset) = value;
			offset += sizeof(T);
			return *this;
		}
		
		void clear()
		{
			uint8_t *cur = buffer->getBuf() + offset;
			memset(cur, 0, size - offset);
		}
		
		uint8_t getOffset() const
		{
			return offset;
		}

		const uint8_t *getBuf() const
		{
			return buffer->getBuf();
		}
	};

	const uint8_t *getBuf() const
	{
		return buf;
	}
	
	uint8_t *getBuf()
	{
		return buf;
	}
};

#endif //HAL1_SERIALISER_H
