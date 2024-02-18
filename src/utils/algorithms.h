#ifndef VS_ALGORITHMS_H
#define VS_ALGORITHMS_H

#include <span>
#include <cstdint>

namespace posnet::utils {

std::uint16_t CalcChecksum(std::span<const std::uint8_t> packet);
std::uint16_t CalcChecksum(std::span<std::uint8_t> packet);

void HostBufferViewToNetwork(std::span<std::int8_t> buffer);
void HostBufferViewToNetwork(std::span<std::uint8_t> buffer);

} //! namespace posnet::utils

#endif //! VS_ALGORITHMS_H