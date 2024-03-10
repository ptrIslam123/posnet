#include "utils/sock_addr_convertor.h"

#include "utils/scoped_lock.h"

#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

namespace posnet::utils {
    
std::string MacAddrToStr(std::span<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES> macAddr)
{
    auto addrStruct = reinterpret_cast<struct ether_addr*>(macAddr.data());
    return ether_ntoa(addrStruct);
}

std::string IpAddrToStr(const std::span<uint8_t, IP_ADDRESS_LENGTH_IN_BYTES> ipAddr)
{
    static std::array<char, INET_ADDRSTRLEN> storage = {0};
    std::memset(storage.data(), 0, storage.size());

    inet_ntop(AF_INET, ipAddr.data(), storage.data(), INET_ADDRSTRLEN);
    return std::string(storage.data(), storage.size());
}

std::string IpAddrToStr(uint32_t ipAddr)
{
    auto addrStruct = reinterpret_cast<struct in_addr*>(&ipAddr);
    return inet_ntoa(*addrStruct);
}

std::string MacAddrToStr(const struct sockaddr& macAddr)
{
    const auto addr = ether_ntoa(reinterpret_cast<const struct ether_addr*>(&macAddr.sa_data));
    return (addr != nullptr ? std::string(addr) : std::string());
}

std::optional<uint32_t> StrToIpAddr(const std::string_view ipAddrStr)
{
    struct in_addr addr;
    if (inet_aton(ipAddrStr.data(), &addr) != 0) {
        return addr.s_addr;
    } else {
        return std::nullopt;
    }
}

std::optional<std::array<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES>> StrToMacAddr(const std::string_view macAddrStr)
{
    std::array<int, MAC_ADDRESS_LENGTH_IN_BYTES> values = {0};
    std::array<uint8_t, MAC_ADDRESS_LENGTH_IN_BYTES> result = {0};
    int count = sscanf(macAddrStr.data(), "%x:%x:%x:%x:%x:%x", 
            &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]);
    if (count != 6) {
        return std::nullopt;
    }

    for (auto i = 0; i < 6; ++i) {
        result[i] = static_cast<uint8_t>(values[i]);
    }
    return result;
}

} //! namespace posnet::utils
