#ifndef VS_STREAM_BUFFER_H
#define VS_STREAM_BUFFER_H

#include "include/base_frame.h"
#include "include/definitions.h"
#include "include/utils/assert.h"

#include <exception>
#include <stdexcept>
#include <span>
#include <vector>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <cstring>

namespace posnet::utils::io::details {

using ByteType = posnet::def::ByteType;
using SizeType = posnet::def::SizeType;

using AllocatorType = std::allocator<ByteType>;

template<typename Allocator = AllocatorType>    
using BufferType = std::vector<ByteType, Allocator>;

using BufferViewType = std::span<ByteType>;
using ConstBufferViewType = std::span<const ByteType>;

template<typename Allocator, typename T>
struct StreamConverter;

template<typename Allocator, typename T>
struct StreamConverter {
    static SizeType Write(BufferType<Allocator>& buffer, const SizeType offset, const T& data)
    {
        static_assert(std::is_trivial<T>::value);
        buffer.resize(offset + sizeof(T));
        auto ptr = buffer.data() + offset;
        std::memcpy(ptr, &data, sizeof(T));
        return sizeof(T);
    }

    static SizeType Read(const ConstBufferViewType buffer, const SizeType offset, T& data)
    {
        static_assert(std::is_trivial<T>::value);
        ASSERTION(buffer.size() >= sizeof(T), std::out_of_range, "buffer size is less than deserialized object")
        auto ptr = buffer.data() + offset;
        std::memcpy(&data, ptr, sizeof(T));
        return sizeof(T);   
    }
};

template<typename Allocator>
struct StreamConverter<Allocator, BaseFrame> {
    static SizeType Write(BufferType<Allocator>& buffer, const SizeType offset, const BaseFrame& data)
    {
        auto frame = data.getAsRawFrameView();
        buffer.resize(offset + frame.size());
        auto ptr = buffer.data() + offset;
        std::memcpy(ptr, frame.data(), frame.size());
        return frame.size();
    }

    static SizeType Read(const ConstBufferViewType buffer, const SizeType offset, BaseFrame& data)
    {
        ASSERTION(false, std::runtime_error, "TODO: Not yet impl!")
        return 0;
    }
};

template<typename Allocator, typename T>
struct StreamConverter<Allocator, std::span<T>> {
    static SizeType Write(BufferType<Allocator>& buffer, const SizeType offset, const std::span<T>& data)
    {
        const auto needTotalSize = (data.size() * sizeof(T)) + sizeof(SizeType);
        buffer.resize(offset + needTotalSize);
        auto ptr = buffer.data() + offset;

        const SizeType dataSize = data.size();
        std::memcpy(ptr, &dataSize, sizeof(SizeType));
        ptr += sizeof(SizeType);

        std::memcpy(ptr, data.data(), dataSize * sizeof(T));
        ptr += dataSize * sizeof(T);
        return (ptr - (buffer.data() + offset));
    }

    static SizeType Read(const ConstBufferViewType buffer, const SizeType offset, std::span<T>& data)
    {
        ASSERTION(buffer.size() >= sizeof(SizeType), std::out_of_range, "buffer size is less than deserialized objects")
        auto ptr = buffer.data() + offset;
        SizeType dataSize = 0;
        std::memcpy(&dataSize, ptr, sizeof(SizeType));
        ptr += sizeof(SizeType);

        ASSERTION(buffer.size() >= sizeof(SizeType) + dataSize, std::out_of_range, "broken buffer size")
        data = std::span<T>(reinterpret_cast<T*>(ptr), static_cast<typename std::span<T>::size_type>(dataSize));
        ptr += dataSize * sizeof(T);
        return (ptr - (buffer.data() + offset));
    }
};

template<typename Allocator, typename T>
struct StreamConverter<Allocator, std::span<const T>> {
    static SizeType Write(BufferType<Allocator>& buffer, const SizeType offset, const std::span<const T>& data)
    {
        const auto needTotalSize = (data.size() * sizeof(T)) + sizeof(SizeType);
        buffer.resize(offset + needTotalSize);
        auto ptr = buffer.data() + offset;

        const SizeType dataSize = data.size();
        std::memcpy(ptr, &dataSize, sizeof(SizeType));
        ptr += sizeof(SizeType);

        std::memcpy(ptr, data.data(), dataSize * sizeof(T));
        ptr += dataSize * sizeof(T);
        return (ptr - (buffer.data() + offset));
    }

    static SizeType Read(const ConstBufferViewType buffer, const SizeType offset, std::span<const T>& data)
    {
        ASSERTION(buffer.size() >= sizeof(SizeType), std::out_of_range, "buffer size is less than deserialized objects")
        auto ptr = buffer.data() + offset;
        SizeType dataSize = 0;
        std::memcpy(&dataSize, ptr, sizeof(SizeType));
        ptr += sizeof(SizeType);

        ASSERTION(buffer.size() >= sizeof(SizeType) + dataSize, std::out_of_range, "broken buffer size")
        data = std::span<const T>(reinterpret_cast<const T*>(ptr), static_cast<typename std::span<const T>::size_type>(dataSize));
        ptr += dataSize * sizeof(T);
        return (ptr - (buffer.data() + offset));
    }
};

template<typename Allocator, typename T>
struct StreamConverter<Allocator, std::vector<T>> {
    static SizeType Write(BufferType<Allocator>& buffer, const SizeType offset, const std::vector<T>& data)
    {
        return StreamConverter<Allocator, std::span<const T>>::Write(buffer, offset, std::span<const T>{ data.data(), data.size() } );
    }
    
    static SizeType Read(const ConstBufferViewType buffer, const SizeType offset, std::vector<T>& data)
    {
        std::span<const T> dataView;
        StreamConverter<Allocator, std::span<const T>>::Read(buffer, offset, dataView);

        data.reserve(dataView.size());
        for (const auto& object : dataView) {
            data.push_back(object);
        }

        return dataView.size() * sizeof(T);
    }
};

} //! namespace posnet::utils::io::details

#endif //! VS_STREAM_BUFFER_H
