#ifndef VS_OSTREAM_BUFFER_H
#define VS_OSTREAM_BUFFER_H

#include <vector>
#include <span>
#include <type_traits>

#include "streambuffer.h"

namespace posnet::utils::io {

/**
* @brief This class using for De-serialization any cpp structure from buffer(raw bytes format).
* @details This class using only for De-serialization.
* @warning This class does not take into account the byte order of the running machine.
* It means that this class doesn't make any assumptions about the byte order (Big Endian (BE)/Little Endian (LE)) of your data.
* You can use this class only for local exchanging raw bytes.
* @example
    IStreamBuffer istreamBuffer;
    ... // from prev example

    OStreamBuffer ostreamBuffer(istreamBuffer.asSpan());
    int v1 = int(); std::string v2; std::vector<int> v3;
    ostreamBuffer >> v1;
    ostreamBuffer >> v2;
    ostreamBuffer >> v3;
*/
template<typename Allocator = std::allocator<posnet::def::ByteType>>
class OStreamBuffer final {
public:
    using ByteType = details::ByteType;
    using SizeType = details::SizeType;
    using AllocatorType = Allocator;
    using BufferType = details::BufferType<Allocator>;
    using BufferViewType = details::BufferViewType;
    using ConstBufferViewType = details::ConstBufferViewType;

    explicit OStreamBuffer(BufferViewType buffer);
    explicit OStreamBuffer(ConstBufferViewType buffer);

    /**
    * @brief Deserialize @object from raw buffer.
    * @tparam T - type of the object.
    * @param object - cpp object for deserialization to raw buffer.
    * @return IOStreamBuffer.
    */
    template<typename T>
    OStreamBuffer& operator>>(T& object);
    ConstBufferViewType asSpan();

private:
    SizeType m_readBytesFromBuffer;
    ConstBufferViewType m_bufferView;
};

template<typename Allocator>
OStreamBuffer<Allocator>::OStreamBuffer(const BufferViewType buffer):
m_readBytesFromBuffer(0),
m_bufferView(ConstBufferViewType{ buffer.data(), buffer.size() })
{}

template<typename Allocator>
OStreamBuffer<Allocator>::OStreamBuffer(const ConstBufferViewType buffer):
m_readBytesFromBuffer(0),
m_bufferView(buffer)
{}

template<typename Allocator>
template<typename T>
OStreamBuffer<Allocator>& OStreamBuffer<Allocator>::operator>>(T& object)
{
    m_readBytesFromBuffer += details::StreamConverter<Allocator, typename std::remove_cv<T>::type>::Read(
        m_bufferView, m_readBytesFromBuffer, object
    );
    return *this;
}

template<typename Allocator>
typename OStreamBuffer<Allocator>::ConstBufferViewType OStreamBuffer<Allocator>::asSpan()
{
    return ConstBufferViewType {
        m_bufferView.data() + m_readBytesFromBuffer,
        static_cast<ConstBufferViewType::size_type>(m_bufferView.size() - m_readBytesFromBuffer)
    };
}

} //! namespace posnet::utils::io

#endif //! VS_OSTREAM_BUFFER_H
