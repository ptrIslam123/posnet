#include "sock_addr_convertor.h"

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

namespace utils::net::posix::low_level {
    
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

std::string MacAddrToStr(struct sockaddr& macAddr)
{
    auto mac = macAddr.sa_data;
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto i = 0; i < 6; ++i) {
        ss << std::setw(2) << static_cast<int>(mac[i]);
        if (i < 5) {
            ss << ":";
        }
    }
    ss << std::dec;
    return ss.str();
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

} //! namespace utils
