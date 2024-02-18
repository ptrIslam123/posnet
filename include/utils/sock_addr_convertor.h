#ifndef VS_SOCK_ADDR_CONVERTOR_H
#define VS_SOCK_ADDR_CONVERTOR_H

#include <string>
#include <string_view>
#include <span>
#include <array>
#include <optional>
#include <cstdint>

#include <net/if.h>

namespace posnet::utils {
    
constexpr auto MAC_ADDRESS_LENGTH_IN_BYTES = 6;
constexpr auto IP_ADDRESS_LENGTH_IN_BYTES = 4;

std::string MacAddrToStr(std::span<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES> macAddr);
std::string MacAddrToStr(struct sockaddr& macAddr);
std::string IpAddrToStr(std::span<uint8_t, IP_ADDRESS_LENGTH_IN_BYTES> ipAddr);
std::string IpAddrToStr(uint32_t ipAddr);

std::optional<std::array<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES>> StrToMacAddr(std::string_view macAddrStr);
std::optional<uint32_t> StrToIpAddr(std::string_view ipAddrStr);

} //! namespace posnet::utils

#endif //! VS_SOCK_ADDR_CONVERTOR_H