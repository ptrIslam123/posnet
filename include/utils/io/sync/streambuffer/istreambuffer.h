#ifndef VS_IO_STREAM_BUFFER_H
#define VS_IO_STREAM_BUFFER_H

#include <vector>
#include <span>
#include <type_traits>

#include "streambuffer.h"

namespace posnet::utils::io {

/**
* @brief This class using for serialization any cpp structure to buffer(raw bytes format).
* @details This class using only for serialization.
* @warning This class does not take into account the byte order of the running machine.
* It means that this class doesn't make any assumptions about the byte order (Big Endian (BE)/Little Endian (LE)) of your data.
* You can use this class only for local exchanging raw bytes.
* @example:
    IStreamBuffer istreamBuffer(0);                 // you can set size of inner buffer, it is optional(like std::vector<T>). 
    istreamBuffer << 10;                            // write int to buffer
    istreamBuffer << std::string("Test string");    // write string to buffer
    istreamBuffer << std::vector<int>{1, 2, 3};     // write vector to buffer
    auto rawBuffer = istreamBuffer.asSpan();        // rawBuffer is reference to raw buffer memory that contains serialized data
    // this raw buffer we may send by socket(after converting raw bytes to BE/LE format) | write to file and etc.
*/
template<typename Allocator = std::allocator<posnet::def::ByteType>>
class IStreamBuffer final {
public: 
    using ByteType = details::ByteType;
    using SizeType = details::SizeType;
    using AllocatorType = Allocator;
    using BufferType = details::BufferType<Allocator>;
    using BufferViewType = details::BufferViewType;
    using ConstBufferViewType = details::ConstBufferViewType;

    explicit IStreamBuffer(SizeType size = 0);

    /**
    * @brief Serialize @object to raw buffer.
    * @tparam T - type of the object.
    * @param object - cpp object for serialization to raw buffer.
    * @return IOStreamBuffer.
    */
    template<typename T>
    IStreamBuffer& operator<<(const T& object);
    ConstBufferViewType asSpan();

private:
    SizeType m_wroteBytesToBuffer;
    BufferType m_buffer;
};

template<typename Allocator>
IStreamBuffer<Allocator>::IStreamBuffer(const SizeType size):
m_wroteBytesToBuffer(0),
m_buffer()
{
    m_buffer.reserve(size);
}

template<typename Allocator>
template<typename T>
IStreamBuffer<Allocator>& IStreamBuffer<Allocator>::operator<<(const T& object)
{
    m_wroteBytesToBuffer += details::StreamConverter<Allocator, typename std::remove_cv<T>::type>::Write(
        m_buffer, m_wroteBytesToBuffer, object
    );
    return *this;
}

template<typename Allocator>
typename IStreamBuffer<Allocator>::ConstBufferViewType IStreamBuffer<Allocator>::asSpan()
{
    return BufferViewType{ m_buffer.data(), m_wroteBytesToBuffer };
}

} //! namespace posnet::utils::io

#endif // !VS_IO_STREAM_BUFFER_H
