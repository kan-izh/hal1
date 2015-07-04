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
		Accessor(const Accessor &src)
		: buffer(src.buffer)
		, offset(src.offset)
		{ }

		Accessor(Buffer *buffer)
		{
			assign(buffer);
			reset();
		}

		void reset()
		{
			offset = 0;
		}

		void assign(Buffer *buffer)
		{
			this->buffer = buffer;
		}

		Buffer *current()
		{
			return buffer;
		}

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

		template<uint8_t otherSize>
		Accessor &append(typename ByteBuffer<otherSize>::Accessor &other)
		{
			uint8_t *cur = buffer->getBuf() + offset;
			memcpy(cur, other.getBuf(), sizeof(uint8_t) * other.getOffset());
			offset += other.getOffset();
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
