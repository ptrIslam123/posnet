#ifndef VS_SOCK_ADDR_CONVERTOR_H
#define VS_SOCK_ADDR_CONVERTOR_H

#include <string>
#include <string_view>
#include <span>
#include <array>
#include <set>
#include <optional>
#include <cstdint>

#include <net/if.h>

#include "definitions.h"

namespace posnet::utils {

constexpr auto MAC_ADDRESS_LENGTH_IN_BYTES = posnet::def::MAC_ADDRESS_LENGTH_IN_BYTES;
constexpr auto IP_ADDRESS_LENGTH_IN_BYTES = posnet::def::IP_ADDRESS_LENGTH_IN_BYTES;

std::string MacAddrToStr(std::span<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES> macAddr);
std::string MacAddrToStr(const struct sockaddr& macAddr);

std::string IpAddrToStr(std::span<uint8_t, IP_ADDRESS_LENGTH_IN_BYTES> ipAddr);
std::string IpAddrToStr(uint32_t ipAddr);

std::optional<std::array<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES>> StrToMacAddr(std::string_view macAddrStr);
std::optional<std::array<std::uint8_t, IP_ADDRESS_LENGTH_IN_BYTES>> StrToIpAddrArray(std::string_view ipAddrStr); 
std::optional<uint32_t> StrToIpAddr(std::string_view ipAddrStr);
std::optional<uint32_t> CountSetBitsInIpAddr(std::string_view ipAddrStr);

std::string DomainNameToIpAddr(std::string_view name);
std::string IpAddrToDomainName(std::string_view ipAddr);

} //! namespace posnet::utils

#endif //! VS_SOCK_ADDR_CONVERTOR_H