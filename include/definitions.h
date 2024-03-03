#ifndef VS_DEFINITIONS_H
#define VS_DEFINITIONS_H

#include <span>
#include <cstdint>

namespace posnet::def {
    
using ByteType = std::uint8_t;
using SizeType = unsigned int;

using BufferViewType = std::span<ByteType>;
using ConstBufferViewType = std::span<const ByteType>;

using RawFrameViewType = BufferViewType;
using ConstRawFrameViewType = ConstBufferViewType;

} // namespace posnet::def

#endif //! VS_DEFINITIONS_H
