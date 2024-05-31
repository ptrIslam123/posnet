#ifndef VS_DEFINITIONS_H
#define VS_DEFINITIONS_H

#include <string_view>
#include <span>
#include <cstdint>

namespace posnet::def {
    
using ByteType = std::uint8_t;
using SizeType = unsigned int;

using BufferViewType = std::span<ByteType>;
using ConstBufferViewType = std::span<const ByteType>;

using RawFrameViewType = BufferViewType;
using ConstRawFrameViewType = ConstBufferViewType;

constexpr std::string_view MAC_BROAD_CAST_ADDR = "FF:FF:FF:FF:FF:FF";
constexpr auto MAC_ADDRESS_LENGTH_IN_BYTES = 6;
constexpr auto IPV4_ADDRESS_LENGTH_IN_BYTES = 4;
constexpr auto IPV6_ADDRESS_LENGTH_IN_BYTES = 6;

} // namespace posnet::def

#endif //! VS_DEFINITIONS_H
