#ifndef VS_ALGORITHMS_H
#define VS_ALGORITHMS_H

#include <span>
#include <set>
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include <cstdint>

namespace posnet::utils {

std::vector<std::string> SplitString(const std::string& str, char delimiter);

std::uint16_t CalcChecksum(std::span<const std::uint8_t> packet);
std::uint16_t CalcChecksum(std::span<std::uint8_t> packet);

void HostBufferViewToNetwork(std::span<std::int8_t> buffer);
void HostBufferViewToNetwork(std::span<std::uint8_t> buffer);

std::set<std::string> GenerateIpAddrRange(std::string_view startIpAddr, std::string_view endIpAddr);
std::set<std::string> GenerateIpAddrRange(std::string_view ipAddr, int cidr/*subnet mask length*/);

} //! namespace posnet::utils

#endif //! VS_ALGORITHMS_H
